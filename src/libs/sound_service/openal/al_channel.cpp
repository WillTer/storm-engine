#include "al_channel.h"

using namespace storm::audio;

ALChannel::ALChannel()  = default;
ALChannel::~ALChannel() = default;

Result ALChannel::play()
{
    return Result::Ok;
}

Result ALChannel::pause()
{
    return Result::Ok;
}

Result ALChannel::stop()
{
    return Result::Ok;
}

Result ALChannel::get_state(ChannelState& state) const
{
    // state = ChannelState::Paused
    return Result::Ok;
}

Result ALChannel::set_playback_position(std::chrono::milliseconds const& pos)
{
    return Result::Ok;
}

Result ALChannel::get_playback_position(std::chrono::milliseconds& pos) const
{
    return Result::Ok;
}

Result ALChannel::set_min_distance(float distance)
{
    return Result::Ok;
}

Result ALChannel::set_max_distance(float distance)
{
    return Result::Ok;
}

Result ALChannel::set_position_3d(std::array<float, 3> const& position)
{
    return Result::Ok;
}

Result ALChannel::set_velocity_3d(std::array<float, 3> const& velocity)
{
    return Result::Ok;
}

Result ALChannel::set_orientation_3d(std::array<float, 3> const& orientation)
{
    return Result::Ok;
}

Result ALChannel::set_volume(float volume_level)
{
    return Result::Ok;
}

Result ALChannel::get_volume(float& volume_level)
{
    return Result::Ok;
}

Result ALChannel::set_pitch(float pitch_level)
{
    return Result::Ok;
}

Result ALChannel::get_pitch(float& pitch_level)
{
    return Result::Ok;
}

Result ALChannel::set_looping(bool flag)
{
    return Result::Ok;
}
