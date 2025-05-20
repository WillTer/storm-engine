#pragma once

#include <array>

#include <AL/al.h>
#include <AL/alc.h>
#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

constexpr size_t MAX_SOURCES_COUNT = 1024;

class ALBackend: virtual public IBackend
{
public:
    ALBackend();
    ~ALBackend() override;

    Result init() override;

    Result create_sound(std::filesystem::path const& file_path, SoundMode sound_mode, std::shared_ptr<ISound>& out) override;

    Result bind_sound(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out) override;

    Result set_listener_position_3d(std::array<float, 3> const& position) override;
    Result set_listener_velocity_3d(std::array<float, 3> const& velocity) override;
    Result set_listener_orientation_3d(std::array<float, 3> const& orientation) override;

    void update() override;

private:
    bool m_is_initialized;

    std::array<unsigned, MAX_SOURCES_COUNT> m_sources;

    ALCdevice*  m_device;
    ALCcontext* m_context;
};

}  // namespace storm::audio
