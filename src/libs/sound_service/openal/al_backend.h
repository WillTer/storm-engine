#pragma once

#include <memory>

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALBackend: virtual public IBackend
{
public:
    ALBackend();
    ~ALBackend() override;

    Result init() override;

    Result
    create_sound(std::filesystem::path const& file_path, SoundMode sound_mode, bool force_stereo, std::shared_ptr<ISound>& out) override;

    Result bind_sound_to_empty_channel(std::shared_ptr<ISound> const& sound, std::weak_ptr<IChannel>& out) override;
    Result release_channel(std::shared_ptr<IChannel> const& channel) override;

    Result set_listener_position_3d(std::array<float, 3> const& position) override;
    Result set_listener_velocity_3d(std::array<float, 3> const& velocity) override;
    Result set_listener_orientation_3d(std::array<float, 6> const& orientation) override;

    void update() override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm::audio
