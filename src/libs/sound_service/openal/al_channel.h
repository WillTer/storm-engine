#pragma once

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALChannel: virtual public IChannel
{
public:
    ALChannel();
    ~ALChannel() override;

    Result play() override;
    Result pause() override;
    Result stop() override;

    Result get_state(AudioState& state) const override;

    Result set_playback_position(std::chrono::milliseconds const& pos) override;
    Result get_playback_position(std::chrono::milliseconds& pos) const override;

    Result set_min_distance(float distance) override;
    Result set_max_distance(float distance) override;

    Result set_position_3d(std::array<float, 3> const& position) override;
    Result set_velocity_3d(std::array<float, 3> const& velocity) override;
    Result set_orientation_3d(std::array<float, 3> const& orientation) override;

    Result set_volume(float volume_level) override;
    Result get_volume(float& volume_level) override;

    Result set_pitch(float pitch_level) override;
    Result get_pitch(float& pitch_level) override;

    Result set_looping(bool flag) override;
};

}  // namespace storm::audio
