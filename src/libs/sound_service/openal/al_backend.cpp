#include "al_backend.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <map>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>
#include <libs/core/core.h>

#include "vorbis/vorbis_decoder.h"
#include "wav/sdl_wav_decoder.h"

#include "al_channel.h"
#include "al_sound.h"
#include "al_utils.h"
#include "converted_data_stream.h"
#include "format_helpers.h"

using namespace storm::audio;

namespace
{

std::unique_ptr<IDataStream> create_compatible_stream(std::filesystem::path const& file_path, IDataStream::Format output_format)
{
    std::string ext = file_path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) { return std::tolower(ch); });

    if (ext == ".ogg") { return std::make_unique<VorbisDecoder>(output_format); }
    if (ext == ".wav") { return std::make_unique<SDLWavDecoder>(); }

    return nullptr;
}

}  // namespace

struct ALBackend::Impl {
    Impl() : m_is_initialized {false}, m_out_format {IDataStream::Format::Unknown}, m_device {nullptr}, m_context {nullptr} {}

    ~Impl()
    {
        if (m_is_initialized) {
            // Remove channels before deinitializing openal
            m_channels.clear();

            alcMakeContextCurrent(nullptr);
            ALC_TRACE_ERRORS(m_device);

            alcDestroyContext(m_context);
            ALC_TRACE_ERRORS(m_device);

            alcCloseDevice(m_device);
        }
    }

    bool init()
    {
        if (m_is_initialized) { return true; }

        m_device = alcOpenDevice(nullptr);  // Default device
        if (m_device == nullptr) { return false; }

        m_context = alcCreateContext(m_device, nullptr);
        ALC_TRACE_ERRORS(m_device);
        if (m_context == nullptr) { return false; }

        alcMakeContextCurrent(m_context);
        ALC_TRACE_ERRORS(m_device);

        m_out_format = convert_format_compatible(IDataStream::Format::Float32);

        m_is_initialized = true;
        return true;
    }

    std::shared_ptr<ISound> create_sound(std::filesystem::path const& file_path, ISound::Flags flags)
    {
        if (!std::filesystem::exists(file_path)) { return nullptr; }

        auto stream = create_compatible_stream(file_path, m_out_format);
        if (!stream || !stream->load_file(file_path)) { return nullptr; }

        auto const channels = is_flag_enabled(flags, ISound::Flags::Spatial3D) ? 1 : stream->get_channels();

        auto stream_wrapper = std::make_unique<ConvertedDataStream>(std::move(stream), channels, m_out_format);
        if (!stream_wrapper || !stream_wrapper->is_valid()) { return nullptr; }

        auto                    sound = std::make_shared<ALSound>(std::move(stream_wrapper), flags);
        std::shared_ptr<ISound> out   = sound;

        // get address of interface ptr as we will compare later with it
        m_sounds.emplace(reinterpret_cast<uintptr_t>(out.get()), sound);

        return out;
    }

    std::weak_ptr<IChannel> bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound)
    {
        if (!sound) { return {}; }

        std::shared_ptr<ALChannel> channel = nullptr;

        auto free_channel = std::find_if(m_channels.begin(), m_channels.end(), [](auto channel) {
            ChannelState state = {};
            channel->get_state(state);
            return state == ChannelState::Stopped;
        });

        if (free_channel != m_channels.end()) {
            channel = *free_channel;
            channel->unbind_sound();
        } else {
            core.Trace("Add new channel, current channels count: %zd", m_channels.size());
            channel = std::make_shared<ALChannel>();
            m_channels.push_back(channel);
        }

        auto sound_id = reinterpret_cast<uintptr_t>(sound.get());
        if (!m_sounds.contains(sound_id)) { return {}; }

        auto al_sound = m_sounds.at(sound_id).lock();
        if (!al_sound) {
            // Remove sound from dictionary if it's not existing anymore
            // TODO: garbage collection (check on update?)
            m_sounds.erase(sound_id);
            return {};
        }

        if (auto res = channel->bind_sound(al_sound); res != Result::Ok) { return {}; }

        return channel;
    }

    void release_channel(std::weak_ptr<IChannel> const& channel)
    {
        auto channel_lock = channel.lock();
        if (!channel_lock) { return; }

        auto it = std::find_if(
            m_channels.begin(), m_channels.end(), [&channel_lock](std::shared_ptr<IChannel> const& ch) { return ch == channel_lock; });

        if (it == m_channels.end()) { return; }

        release_channel(std::distance(m_channels.begin(), it));
    }

    void set_listener_position_3d(std::array<float, 3> const& position)
    {
        alListenerfv(AL_POSITION, position.data());
        AL_TRACE_ERRORS();
    }

    void set_listener_velocity_3d(std::array<float, 3> const& velocity)
    {
        alListenerfv(AL_VELOCITY, velocity.data());
        AL_TRACE_ERRORS();
    }

    void set_listener_orientation_3d(std::array<float, 6> const& orientation)
    {
        alListenerfv(AL_ORIENTATION, orientation.data());
        AL_TRACE_ERRORS();
    }

    void update()
    {
        for (auto& channel: m_channels) {
            channel->internal_update();
        }
    }

    void release_channel(size_t idx)
    {
        if (idx == m_channels.size()) { return; }

        m_channels[idx]->stop();
        m_channels[idx]->unbind_sound();
    }

    bool m_is_initialized;

    std::vector<std::shared_ptr<ALChannel>>     m_channels;
    std::map<uintptr_t, std::weak_ptr<ALSound>> m_sounds;

    IDataStream::Format m_out_format;

    ALCdevice*  m_device;
    ALCcontext* m_context;
};

ALBackend::ALBackend() : m_impl {std::make_unique<Impl>()} {}

ALBackend::~ALBackend() = default;

bool ALBackend::init()
{
    return m_impl->init();
}

std::shared_ptr<ISound> ALBackend::create_sound(std::filesystem::path const& file_path, ISound::Flags flags)
{
    if (!m_impl->m_is_initialized) { return nullptr; }

    return m_impl->create_sound(file_path, flags);
}

std::weak_ptr<IChannel> ALBackend::bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound)
{
    if (!m_impl->m_is_initialized) { return {}; }

    return m_impl->bind_sound_to_empty_channel(sound);
}

void ALBackend::release_channel(std::weak_ptr<IChannel> const& channel)
{
    if (!m_impl->m_is_initialized) { return; }

    m_impl->release_channel(channel);
}

void ALBackend::set_listener_position_3d(std::array<float, 3> const& position)
{
    if (!m_impl->m_is_initialized) { return; }

    std::array<float, 3> position_lh = position;
    // flip z coordinate
    position_lh[2] = -position_lh[2];
    m_impl->set_listener_position_3d(position_lh);
}

void ALBackend::set_listener_velocity_3d(std::array<float, 3> const& velocity)
{
    if (!m_impl->m_is_initialized) { return; }

    std::array<float, 3> velocity_lh = velocity;
    // flip z coordinate
    velocity_lh[2] = -velocity_lh[2];
    m_impl->set_listener_velocity_3d(velocity_lh);
}

void ALBackend::set_listener_orientation_3d(std::array<float, 6> const& orientation)
{
    if (!m_impl->m_is_initialized) { return; }

    std::array<float, 6> orientation_lh = orientation;
    // flip z coordinate
    orientation_lh[2] = -orientation_lh[2];
    orientation_lh[5] = -orientation_lh[5];
    m_impl->set_listener_orientation_3d(orientation_lh);
}

void ALBackend::update()
{
    m_impl->update();
}
