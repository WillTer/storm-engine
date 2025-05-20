#include "al_channel.h"

#include "al_sound.h"
#include "al_utils.h"

using namespace storm::audio;

struct ALChannel::Impl {
    Impl() : is_looping {false}
    {
        alGenSources(1, &source);
        AL_TRACE_ERRORS();
    }

    ~Impl()
    {
        // Unbind all buffers from source
        alSourcei(source, AL_BUFFER, 0);
        AL_TRACE_ERRORS();

        alDeleteSources(1, &source);
        AL_TRACE_ERRORS();
    }

    Result play()
    {
        if (!sound) { return Result::ErrChannelIsEmpty; }

        alSourcePlay(source);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result pause()
    {
        if (!sound) { return Result::ErrChannelIsEmpty; }

        alSourcePause(source);
        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result stop()
    {
        if (!sound) { return Result::ErrChannelIsEmpty; }

        alSourceStop(source);
        AL_TRACE_ERRORS();

        unbind_sound();

        return Result::Ok;
    }

    Result get_state(ChannelState& state) const
    {
        if (!sound) { return Result::ErrChannelIsEmpty; }

        ALint al_state = 0;
        alGetSourcei(source, AL_SOURCE_STATE, &al_state);

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
        if (!sound) { return Result::ErrChannelIsEmpty; }

        return Result::Ok;
    }

    Result get_playback_position(std::chrono::milliseconds& pos) const
    {
        if (!sound) { return Result::ErrChannelIsEmpty; }

        return Result::Ok;
    }

    Result set_min_distance(float distance)
    {
        return Result::Ok;
    }

    Result set_max_distance(float distance)
    {
        return Result::Ok;
    }

    Result set_position_3d(std::array<float, 3> const& position)
    {
        return Result::Ok;
    }

    Result set_velocity_3d(std::array<float, 3> const& velocity)
    {
        return Result::Ok;
    }

    Result set_orientation_3d(std::array<float, 3> const& orientation)
    {
        return Result::Ok;
    }

    Result set_volume(float volume_level)
    {
        return Result::Ok;
    }

    Result get_volume(float& volume_level)
    {
        return Result::Ok;
    }

    Result set_pitch(float pitch_level)
    {
        return Result::Ok;
    }

    Result get_pitch(float& pitch_level)
    {
        return Result::Ok;
    }

    Result set_looping(bool flag)
    {
        is_looping = flag;
        if (sound) { sound->set_looping(source, flag); }

        return Result::Ok;
    }

    Result bind_sound(std::shared_ptr<ALSound> const& sound)
    {
        this->sound = sound;
        sound->bind_buffers_to_source(source, is_looping);

        return Result::Ok;
    }

    Result unbind_sound()
    {
        alSourcei(source, AL_BUFFER, 0);
        AL_TRACE_ERRORS();

        sound.reset();
        return Result::Ok;
    }

    bool internal_update() const
    {
        ChannelState state = {};
        get_state(state);
        if (!sound || state == ChannelState::Stopped) { return false; }

        SoundMode mode = {};
        sound->get_sound_mode(mode);

        // No need to update buffer if stream is on pause or there is no stream at all
        if (state == ChannelState::Paused || mode == SoundMode::WholeFile) { return true; }

        ALint processed = 0;
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        AL_TRACE_ERRORS();

        for (ALint i = 0; i < processed; ++i) {
            ALuint buffer = 0;
            alSourceUnqueueBuffers(source, 1, &buffer);
            AL_TRACE_ERRORS();

            if (sound->push_next_data(buffer)) {
                alSourceQueueBuffers(source, 1, &buffer);
                AL_TRACE_ERRORS();
            }
        }

        return true;
    }

    unsigned source;

    bool is_looping;

    std::shared_ptr<ALSound> sound;
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

Result ALChannel::set_orientation_3d(std::array<float, 3> const& orientation)
{
    return m_impl->set_orientation_3d(orientation);
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

bool ALChannel::internal_update()
{
    return m_impl->internal_update();
}
