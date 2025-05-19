#pragma once

#include <array>
#include <chrono>
#include <filesystem>

namespace storm::audio
{

enum class Result : int32_t {
    Ok                = 0,
    ErrFileNotFound   = -1,
    ErrChannelIsEmpty = -2,

    ErrUnknown = std::numeric_limits<int32_t>::min(),
};

class IChannel
{
public:
    virtual ~IChannel() = default;

    enum class AudioState { None, Playing, Paused, Stopped };

    virtual Result play()  = 0;
    virtual Result pause() = 0;
    virtual Result stop()  = 0;

    virtual Result get_state(AudioState& state) const = 0;

    virtual Result set_playback_position(std::chrono::milliseconds const& pos) = 0;
    virtual Result get_playback_position(std::chrono::milliseconds& pos) const = 0;

    virtual Result set_min_distance(float distance) = 0;
    virtual Result set_max_distance(float distance) = 0;

    virtual Result set_position_3d(std::array<float, 3> const& position)       = 0;
    virtual Result set_velocity_3d(std::array<float, 3> const& velocity)       = 0;
    virtual Result set_orientation_3d(std::array<float, 3> const& orientation) = 0;

    virtual Result set_volume(float volume_level)  = 0;
    virtual Result get_volume(float& volume_level) = 0;

    virtual Result set_pitch(float pitch_level)  = 0;
    virtual Result get_pitch(float& pitch_level) = 0;

    virtual Result set_looping(bool flag) = 0;
};

class ISound
{
public:
    virtual ~ISound() = default;
};

class IBackend
{
public:
    virtual ~IBackend() = default;

    virtual Result init() = 0;

    virtual Result create_sound(std::filesystem::path const& file, std::shared_ptr<ISound>& out)        = 0;
    virtual Result create_sound_stream(std::filesystem::path const& file, std::shared_ptr<ISound>& out) = 0;

    virtual Result bind_sound(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out) = 0;

    virtual Result set_listener_position_3d(std::array<float, 3> const& position)       = 0;
    virtual Result set_listener_velocity_3d(std::array<float, 3> const& velocity)       = 0;
    virtual Result set_listener_orientation_3d(std::array<float, 3> const& orientation) = 0;

    virtual void update() = 0;
};

}  // namespace storm::audio
