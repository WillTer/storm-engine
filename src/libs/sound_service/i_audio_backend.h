#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <vector>

#include "enum_flags.h"

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

    virtual Result set_position_3d(std::array<float, 3> const& position)     = 0;
    virtual Result set_velocity_3d(std::array<float, 3> const& velocity)     = 0;
    virtual Result set_direction_3d(std::array<float, 3> const& orientation) = 0;

    virtual Result set_volume(float volume_level)  = 0;
    virtual Result get_volume(float& volume_level) = 0;

    virtual Result set_pitch(float pitch_level)  = 0;
    virtual Result get_pitch(float& pitch_level) = 0;

    virtual Result set_looping(bool flag) = 0;
};

class IDataStream
{
public:
    enum class Format {
        Unknown,
        Int8,
        Int16,
        Int32,
        UInt8,
        UInt16,
        Float32,
    };

    virtual ~IDataStream() = default;

    virtual bool load_file(std::filesystem::path const& file_path) = 0;
    virtual bool load_memory(std::vector<uint8_t> const& mem)      = 0;

    virtual bool is_valid() = 0;

    virtual int get_channels() const    = 0;
    virtual int get_sample_rate() const = 0;

    virtual IDataStream::Format get_data_format() const = 0;

    virtual size_t get_samples(std::vector<uint8_t>& buffer, size_t sample_count) = 0;
    virtual size_t get_samples_all(std::vector<uint8_t>& buffer)                  = 0;

    virtual void seek_start() = 0;
};

class ISound
{
public:
    enum class Flags : uint8_t {
        None      = 0,
        Stream    = 1 << 0,
        Stereo2D  = 1 << 1,
        Spatial3D = 1 << 2,
    };

    virtual ~ISound() = default;

    virtual Flags get_flags() const = 0;
};

class IBackend
{
public:
    virtual ~IBackend() = default;

    virtual bool init() = 0;

    virtual std::shared_ptr<ISound> create_sound(std::filesystem::path const& file_path, ISound::Flags flags) = 0;

    virtual std::weak_ptr<IChannel> bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound) = 0;
    virtual void                    release_channel(std::weak_ptr<IChannel> const& channel)           = 0;

    virtual void set_listener_position_3d(std::array<float, 3> const& position)       = 0;
    virtual void set_listener_velocity_3d(std::array<float, 3> const& velocity)       = 0;
    virtual void set_listener_orientation_3d(std::array<float, 6> const& orientation) = 0;

    virtual void update() = 0;
};

}  // namespace storm::audio

namespace storm::type_traits
{

template <>
struct is_flag<audio::ISound::Flags>: std::true_type {
};

}  // namespace storm::type_traits
