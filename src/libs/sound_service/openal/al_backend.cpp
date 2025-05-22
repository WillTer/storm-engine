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

Result get_decoder(std::filesystem::path const& file_path, ISound::Flags flags, std::shared_ptr<IDecoder>& out)
{
    std::string ext = file_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return std::tolower(ch); });

    if (ext == ".ogg") {
        out = std::make_shared<VorbisDecoder>(flags);
    } else if (ext == ".wav") {
        out = std::make_shared<SDLWavDecoder>(flags);
    } else {
        return Result::ErrFileFormatNotSupported;
    }

    return Result::Ok;
}

}  // namespace

struct ALBackend::Impl {
    Impl() : is_initialized {false}, device {nullptr}, context {nullptr} {}

    ~Impl()
    {
        if (is_initialized) {
            // Remove channels before deinitializing openal
            channels.clear();

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

    Result create_sound(std::filesystem::path const& file_path, ISound::Flags flags, std::shared_ptr<ISound>& out)
    {
        if (!std::filesystem::exists(file_path)) { return Result::ErrFileNotFound; }

        std::shared_ptr<IDecoder> decoder = nullptr;

        if (auto res = get_decoder(file_path, flags, decoder); res != Result::Ok) { return res; }
        if (auto res = decoder->init(file_path); res != Result::Ok) { return res; }

        auto sound = std::make_shared<ALSound>(decoder, flags);
        out        = sound;

        // get address of interface ptr as we will compare later with it
        sounds.emplace(reinterpret_cast<uintptr_t>(out.get()), sound);

        return Result::Ok;
    }

    Result bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound, std::weak_ptr<IChannel>& out)
    {
        if (!sound) { return Result::ErrInvalidArgument; }

        std::shared_ptr<ALChannel> channel = nullptr;

        auto free_channel = std::find_if(channels.begin(), channels.end(), [](auto channel) {
            ChannelState state = {};
            channel->get_state(state);
            return state == ChannelState::Stopped;
        });

        if (free_channel != channels.end()) {
            channel = *free_channel;
            channel->unbind_sound();
        } else {
            core.Trace("Add new channel, current channels count: %zd", channels.size());
            channel = std::make_shared<ALChannel>();
            channels.push_back(channel);
        }

        auto sound_id = reinterpret_cast<uintptr_t>(sound.get());
        if (!sounds.contains(sound_id)) { return Result::ErrInvalidArgument; }

        auto al_sound = sounds.at(sound_id).lock();
        if (!al_sound) {
            // Remove sound from dictionary if it's not existing anymore
            // TODO: garbage collection (check on update?)
            sounds.erase(sound_id);
            return Result::ErrInvalidArgument;
        }

        if (auto res = channel->bind_sound(al_sound); res != Result::Ok) { return res; }

        out = channel;

        return Result::Ok;
    }

    Result release_channel(std::shared_ptr<IChannel> const& channel)
    {
        if (!channel) { return Result::ErrInvalidArgument; }

        auto it = std::find_if(channels.begin(), channels.end(), [&channel](std::shared_ptr<IChannel> const& ch) { return ch == channel; });

        if (it == channels.end()) { return Result::ErrInvalidArgument; }

        return release_channel(std::distance(channels.begin(), it));
    }

    Result set_listener_position_3d(std::array<float, 3> const& position)
    {
        alListenerfv(AL_POSITION, position.data());
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_listener_velocity_3d(std::array<float, 3> const& velocity)
    {
        alListenerfv(AL_VELOCITY, velocity.data());
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_listener_orientation_3d(std::array<float, 6> const& orientation)
    {
        alListenerfv(AL_ORIENTATION, orientation.data());
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    void update()
    {
        for (auto& channel: channels) {
            channel->internal_update();
        }
    }

    Result release_channel(size_t idx)
    {
        if (idx == channels.size()) { return Result::ErrChannelIsEmpty; }

        channels[idx]->stop();
        return channels[idx]->unbind_sound();
    }

    bool is_initialized;

    std::vector<std::shared_ptr<ALChannel>>     channels;
    std::map<uintptr_t, std::weak_ptr<ALSound>> sounds;

    ALCdevice*  device;
    ALCcontext* context;
};

ALBackend::ALBackend() : m_impl {std::make_unique<Impl>()} {}

ALBackend::~ALBackend() = default;

Result ALBackend::init()
{
    return m_impl->init();
}

Result ALBackend::create_sound(std::filesystem::path const& file_path, ISound::Flags flags, std::shared_ptr<ISound>& out)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->create_sound(file_path, flags, out);
}

Result ALBackend::bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound, std::weak_ptr<IChannel>& out)
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

    std::array<float, 3> position_lh = position;
    // flip z coordinate
    position_lh[2] = -position_lh[2];
    return m_impl->set_listener_position_3d(position_lh);
}

Result ALBackend::set_listener_velocity_3d(std::array<float, 3> const& velocity)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    std::array<float, 3> velocity_lh = velocity;
    // flip z coordinate
    velocity_lh[2] = -velocity_lh[2];
    return m_impl->set_listener_velocity_3d(velocity_lh);
}

Result ALBackend::set_listener_orientation_3d(std::array<float, 6> const& orientation)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    std::array<float, 6> orientation_lh = orientation;
    // flip z coordinate
    orientation_lh[2] = -orientation_lh[2];
    orientation_lh[5] = -orientation_lh[5];
    return m_impl->set_listener_orientation_3d(orientation_lh);
}

void ALBackend::update()
{
    m_impl->update();
}
