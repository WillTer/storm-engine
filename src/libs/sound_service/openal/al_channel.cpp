#include "al_channel.h"

#include "al_sound.h"
#include "al_utils.h"

using namespace storm::audio;

struct ALChannel::Impl {
    Impl() : m_is_looping {false}
    {
        alGenSources(1, &m_source);
        AL_TRACE_ERRORS();
    }

    ~Impl()
    {
        alSourceStop(m_source);
        AL_TRACE_ERRORS();

        // Unbind all buffers from source
        unbind_sound();

        alDeleteSources(1, &m_source);
        AL_TRACE_ERRORS();
    }

    Result play()
    {
        if (!m_sound) { return Result::ErrChannelIsEmpty; }

        alSourcePlay(m_source);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result pause()
    {
        if (!m_sound) { return Result::ErrChannelIsEmpty; }

        alSourcePause(m_source);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result stop()
    {
        if (!m_sound) { return Result::ErrChannelIsEmpty; }

        alSourceStop(m_source);
        AL_TRACE_ERRORS();

        unbind_sound();

        return Result::Ok;
    }

    Result get_state(ChannelState& state) const
    {
        ALint al_state = 0;
        alGetSourcei(m_source, AL_SOURCE_STATE, &al_state);
        AL_TRACE_ERRORS();

        switch (al_state) {
        case AL_PLAYING: state = ChannelState::Playing; break;
        case AL_PAUSED: state = ChannelState::Paused; break;
        case AL_STOPPED: state = ChannelState::Stopped; break;
        default: state = ChannelState::None; break;
        }

        return Result::Ok;
    }

    Result set_playback_position(std::chrono::milliseconds const& pos)
    {
        if (!m_sound) { return Result::ErrChannelIsEmpty; }

        return Result::Ok;
    }

    Result get_playback_position(std::chrono::milliseconds& pos) const
    {
        if (!m_sound) { return Result::ErrChannelIsEmpty; }

        return Result::Ok;
    }

    Result set_min_distance(float distance)
    {
        return Result::Ok;
    }

    Result set_max_distance(float distance)
    {
        alSourcef(m_source, AL_MAX_DISTANCE, distance);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_position_3d(std::array<float, 3> const& position) const
    {
        alSource3f(m_source, AL_POSITION, position[0], position[1], -position[2]);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_velocity_3d(std::array<float, 3> const& velocity) const
    {
        alSource3f(m_source, AL_VELOCITY, velocity[0], velocity[1], -velocity[2]);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_direction_3d(std::array<float, 3> const& direction) const
    {
        alSource3f(m_source, AL_DIRECTION, direction[0], direction[1], -direction[2]);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_volume(float volume_level) const
    {
        alSourcef(m_source, AL_GAIN, volume_level);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result get_volume(float& volume_level) const
    {
        alGetSourcef(m_source, AL_GAIN, &volume_level);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_pitch(float pitch_level) const
    {
        alSourcef(m_source, AL_PITCH, pitch_level);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result get_pitch(float& pitch_level) const
    {
        alGetSourcef(m_source, AL_PITCH, &pitch_level);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_looping(bool flag)
    {
        m_is_looping = flag;
        if (m_sound) { m_sound->set_looping(m_source, flag); }

        return Result::Ok;
    }

    Result bind_sound(std::shared_ptr<ALSound> const& sound)
    {
        this->m_sound = sound;
        sound->bind_buffers_to_source(m_source, m_is_looping);

        if (is_flag_enabled(sound->get_flags(), ISound::Flags::Stereo2D) && sound->get_channels() == 1) {
            alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
            AL_TRACE_ERRORS();

            alSourcei(m_source, AL_REFERENCE_DISTANCE, 1);
            AL_TRACE_ERRORS();

            // Update 3D data
            set_position_3d({0, 0, 1});
            set_direction_3d({});
            set_velocity_3d({});
        }

        return Result::Ok;
    }

    Result unbind_sound()
    {
        alSourcei(m_source, AL_BUFFER, 0);
        AL_TRACE_ERRORS();

        if (m_sound) {
            m_sound->unbind_source(m_source);
            m_sound.reset();
        }

        return Result::Ok;
    }

    void internal_update() const
    {
        if (!m_sound) { return; }

        ChannelState state = {};
        get_state(state);

        if (state == ChannelState::Paused) { return; }

        if (is_flag_enabled(m_sound->get_flags(), ISound::Flags::Stream)) { update_stream(); }
    }

    void update_stream() const
    {
        ALint processed = 0;
        alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
        AL_TRACE_ERRORS();

        for (ALint i = 0; i < processed; ++i) {
            ALuint buffer = 0;
            alSourceUnqueueBuffers(m_source, 1, &buffer);
            AL_TRACE_ERRORS();

            if (m_sound->push_next_data(buffer, m_is_looping)) {
                alSourceQueueBuffers(m_source, 1, &buffer);
                AL_TRACE_ERRORS();
            }
        }
    }

    unsigned m_source;
    bool     m_is_looping;

    std::shared_ptr<ALSound> m_sound;
};

ALChannel::ALChannel() : m_impl {std::make_unique<Impl>()} {}

ALChannel::~ALChannel() = default;

Result ALChannel::play()
{
    return m_impl->play();
}

Result ALChannel::pause()
{
    return m_impl->pause();
}

Result ALChannel::stop()
{
    return m_impl->stop();
}

Result ALChannel::get_state(ChannelState& state) const
{
    return m_impl->get_state(state);
}

Result ALChannel::set_playback_position(std::chrono::milliseconds const& pos)
{
    return m_impl->set_playback_position(pos);
}

Result ALChannel::get_playback_position(std::chrono::milliseconds& pos) const
{
    return m_impl->get_playback_position(pos);
}

Result ALChannel::set_min_distance(float distance)
{
    return m_impl->set_min_distance(distance);
}

Result ALChannel::set_max_distance(float distance)
{
    return m_impl->set_max_distance(distance);
}

Result ALChannel::set_position_3d(std::array<float, 3> const& position)
{
    return m_impl->set_position_3d(position);
}

Result ALChannel::set_velocity_3d(std::array<float, 3> const& velocity)
{
    return m_impl->set_velocity_3d(velocity);
}

Result ALChannel::set_direction_3d(std::array<float, 3> const& orientation)
{
    return m_impl->set_direction_3d(orientation);
}

Result ALChannel::set_volume(float volume_level)
{
    return m_impl->set_volume(volume_level);
}

Result ALChannel::get_volume(float& volume_level)
{
    return m_impl->get_volume(volume_level);
}

Result ALChannel::set_pitch(float pitch_level)
{
    return m_impl->set_pitch(pitch_level);
}

Result ALChannel::get_pitch(float& pitch_level)
{
    return m_impl->get_pitch(pitch_level);
}

Result ALChannel::set_looping(bool flag)
{
    return m_impl->set_looping(flag);
}

Result ALChannel::bind_sound(std::shared_ptr<ALSound> const& sound)
{
    return m_impl->bind_sound(sound);
}

Result ALChannel::unbind_sound()
{
    return m_impl->unbind_sound();
}

void ALChannel::internal_update()
{
    m_impl->internal_update();
}
