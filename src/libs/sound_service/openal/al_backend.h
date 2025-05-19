#pragma once

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALBackend: virtual public IBackend
{
public:
    ALBackend();
    ~ALBackend() override;

    Result init() override;

    Result create_sound(std::filesystem::path const& file, std::shared_ptr<ISound>& out) override;
    Result create_sound_stream(std::filesystem::path const& file, std::shared_ptr<ISound>& out) override;

    Result bind_sound(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out) override;

    Result set_listener_position_3d(std::array<float, 3> const& position) override;
    Result set_listener_velocity_3d(std::array<float, 3> const& velocity) override;
    Result set_listener_orientation_3d(std::array<float, 3> const& orientation) override;

    void update() override;
};

}  // namespace storm::audio
