#include "al_backend.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <map>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>
#include <libs/core/core.h>
#include <libs/sound_service/vorbis/vorbis_decoder.h>
#include <libs/sound_service/wav/sdl_wav_decoder.h>

#include "al_channel.h"
#include "al_sound.h"
#include "al_utils.h"

using namespace storm::audio;

namespace
{

Result get_decoder(std::filesystem::path const& file_path, std::shared_ptr<IDecoder>& out)
{
    std::string ext = file_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return std::tolower(ch); });

    if (ext == ".ogg") {
        out = std::make_shared<VorbisDecoder>();
    } else if (ext == ".wav") {
        out = std::make_shared<SDLWavDecoder>();
    } else {
        return Result::ErrFileFormatNotSupported;
    }

    return Result::Ok;
}

constexpr size_t MAX_SOURCES_COUNT = 128;

}  // namespace

struct ALBackend::Impl {
    Impl() : is_initialized {false}, channels_in_use {0}, device {nullptr}, context {nullptr} {}

    ~Impl()
    {
        if (is_initialized) {
            // Remove channels before deinitializing openal
            for (size_t i = 0; i < channels_in_use; ++i) {
                channels[i].reset();
            }

            alcMakeContextCurrent(nullptr);
            ALC_TRACE_ERRORS(device);

            alcDestroyContext(context);
            ALC_TRACE_ERRORS(device);

            alcCloseDevice(device);
        }
    }

    Result init()
    {
        if (is_initialized) { return Result::Ok; }

        device = alcOpenDevice(nullptr);  // Default device
        if (device == nullptr) { return Result::ErrInternal; }

        context = alcCreateContext(device, nullptr);
        ALC_TRACE_ERRORS(device);
        if (context == nullptr) { return Result::ErrInternal; }

        alcMakeContextCurrent(context);
        ALC_TRACE_ERRORS(device);

        is_initialized = true;
        return Result::Ok;
    }

    Result create_sound(std::filesystem::path const& file_path, SoundMode sound_mode, std::shared_ptr<ISound>& out)
    {
        if (!std::filesystem::exists(file_path)) { return Result::ErrFileNotFound; }

        std::shared_ptr<IDecoder> decoder = nullptr;

        if (auto res = get_decoder(file_path, decoder); res != Result::Ok) { return res; }
        if (auto res = decoder->init(file_path); res != Result::Ok) { return res; }

        auto sound = std::make_shared<ALSound>(decoder, sound_mode);
        out        = sound;
        sounds.emplace(reinterpret_cast<uintptr_t>(out.get()), sound);

        return Result::Ok;
    }

    Result bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out)
    {
        if (free_channels.empty() && channels_in_use >= channels.size()) { return Result::ErrNoEmptyChannels; }
        if (!sound) { return Result::ErrInvalidArgument; }

        std::shared_ptr<ALChannel> channel = nullptr;

        auto free_channel = std::find_if(free_channels.begin(), free_channels.end(), [this](size_t free_channel) {
            ChannelState state = {};
            channels[free_channel]->get_state(state);
            return state == ChannelState::Stopped;
        });

        if (free_channel != free_channels.end()) {
            channel = channels[*free_channel];
            free_channels.erase(free_channel);
        } else {
            channel                     = std::make_shared<ALChannel>();
            channels[channels_in_use++] = channel;
        }

        auto sound_id = reinterpret_cast<uintptr_t>(sound.get());
        if (!sounds.contains(sound_id)) { return Result::ErrInvalidArgument; }

        auto al_sound = sounds.at(sound_id).lock();
        if (!al_sound) { return Result::ErrInvalidArgument; }

        if (auto res = channel->bind_sound(al_sound); res != Result::Ok) { return res; }

        out = channel;

        return Result::Ok;
    }

    Result release_channel(std::shared_ptr<IChannel> const& channel)
    {
        if (!channel) { return Result::ErrInvalidArgument; }

        size_t idx = channels_in_use;
        for (size_t i = 0; i < channels_in_use; ++i) {
            if (channel == channels[i]) {
                idx = i;
                break;
            }
        }

        return release_channel(idx);
    }

    Result set_listener_position_3d(std::array<float, 3> const& position)
    {
        return Result::Ok;
    }

    Result set_listener_velocity_3d(std::array<float, 3> const& velocity)
    {
        return Result::Ok;
    }

    Result set_listener_orientation_3d(std::array<float, 3> const& orientation)
    {
        return Result::Ok;
    }

    void update()
    {
        for (size_t i = 0; i < channels_in_use; ++i) {
            if (!channels[i]->internal_update()) { release_channel(i); }
        }
    }

    Result release_channel(size_t idx)
    {
        if (idx == channels_in_use) { return Result::ErrChannelIsEmpty; }

        free_channels.push_back(idx);

        channels[idx]->stop();
        return channels[idx]->unbind_sound();
    }

    bool is_initialized;

    std::array<std::shared_ptr<ALChannel>, MAX_SOURCES_COUNT> channels;
    std::map<uintptr_t, std::weak_ptr<ALSound>>               sounds;

    std::vector<size_t> free_channels;

    size_t channels_in_use;

    ALCdevice*  device;
    ALCcontext* context;
};

ALBackend::ALBackend() : m_impl {std::make_unique<Impl>()} {}

ALBackend::~ALBackend() = default;

Result ALBackend::init()
{
    return m_impl->init();
}

Result ALBackend::create_sound(std::filesystem::path const& file_path, SoundMode sound_mode, std::shared_ptr<ISound>& out)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->create_sound(file_path, sound_mode, out);
}

Result ALBackend::bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->bind_sound_to_empty_channel(sound, out);
}

Result ALBackend::release_channel(std::shared_ptr<IChannel> const& channel)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->release_channel(channel);
}

Result ALBackend::set_listener_position_3d(std::array<float, 3> const& position)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->set_listener_position_3d(position);
}

Result ALBackend::set_listener_velocity_3d(std::array<float, 3> const& velocity)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->set_listener_velocity_3d(velocity);
}

Result ALBackend::set_listener_orientation_3d(std::array<float, 3> const& orientation)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->set_listener_orientation_3d(orientation);
}

void ALBackend::update()
{
    m_impl->update();
}
