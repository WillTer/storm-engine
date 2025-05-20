#pragma once

#include <array>
#include <chrono>
#include <filesystem>

namespace storm::audio
{

enum class Result : int32_t {
    Ok                        = 0,
    ErrNotInitialized         = -1,
    ErrInvalidArgument        = -2,
    ErrFileNotFound           = -3,
    ErrFileOpenFailed         = -4,
    ErrFileFormatInvalid      = -5,
    ErrFileFormatNotSupported = -6,
    ErrChannelIsEmpty         = -7,
    ErrNoEmptyChannels        = -8,
    ErrInternal               = -9,

    ErrUnknown = std::numeric_limits<int32_t>::min(),
};

enum class ChannelState { None, Playing, Paused, Stopped };

enum class SoundMode { WholeFile, Stream };

enum class SoundFormat {
    Mono8,
    Mono16,
    Stereo8,
    Stereo16,
};

class IChannel
{
public:
    virtual ~IChannel() = default;

    virtual Result play()  = 0;
    virtual Result pause() = 0;
    virtual Result stop()  = 0;

    virtual Result get_state(ChannelState& state) const = 0;

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

class IDecoder
{
public:
    virtual ~IDecoder() = default;

    virtual Result init(std::filesystem::path const& file_path) = 0;

    virtual Result get_channels(int& channels)           = 0;
    virtual Result get_sample_rate(int& sample_rate)     = 0;
    virtual Result get_sound_format(SoundFormat& format) = 0;

    virtual Result get_pcm_data(std::vector<uint8_t>& data, bool read_until_end) = 0;

    virtual Result seek_start() = 0;
};

class ISound
{
public:
    virtual ~ISound() = default;

    virtual Result get_sound_mode(SoundMode& mode) = 0;
};

class IBackend
{
public:
    virtual ~IBackend() = default;

    virtual Result init() = 0;

    virtual Result create_sound(std::filesystem::path const& file_path, SoundMode mode, std::shared_ptr<ISound>& out) = 0;

    virtual Result bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out) = 0;
    virtual Result release_channel(std::shared_ptr<IChannel> const& channel)                                         = 0;

    virtual Result set_listener_position_3d(std::array<float, 3> const& position)       = 0;
    virtual Result set_listener_velocity_3d(std::array<float, 3> const& velocity)       = 0;
    virtual Result set_listener_orientation_3d(std::array<float, 3> const& orientation) = 0;

    virtual void update() = 0;
};

}  // namespace storm::audio
