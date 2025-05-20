#include "al_backend.h"

#include <fstream>

#include <libs/core/core.h>
#include <libs/sound_service/vorbis/vorbis_decoder.h>

#include "al_channel.h"
#include "al_sound.h"
#include "al_utils.h"

using namespace storm::audio;

ALBackend::ALBackend() : m_is_initialized {false}, m_device {nullptr}, m_context {nullptr} {}
ALBackend::~ALBackend()
{
    if (m_is_initialized) {
        // Unbind all buffers from sources first
        for (auto const& source: m_sources) {
            alSourcei(source, AL_BUFFER, 0);
            AL_TRACE_ERRORS();
        }

        alDeleteSources(static_cast<int>(m_sources.size()), m_sources.data());
        AL_TRACE_ERRORS();

        alcMakeContextCurrent(nullptr);
        ALC_TRACE_ERRORS(m_device);

        alcDestroyContext(m_context);
        ALC_TRACE_ERRORS(m_device);

        alcCloseDevice(m_device);
    }
}

Result ALBackend::init()
{
    if (m_is_initialized) { return Result::Ok; }

    m_device = alcOpenDevice(nullptr);  // Default device
    if (m_device == nullptr) { return Result::ErrInternal; }

    m_context = alcCreateContext(m_device, nullptr);
    ALC_TRACE_ERRORS(m_device);
    if (m_context == nullptr) { return Result::ErrInternal; }

    alcMakeContextCurrent(m_context);
    ALC_TRACE_ERRORS(m_device);

    alGenSources(static_cast<int>(m_sources.size()), m_sources.data());
    AL_TRACE_ERRORS();

    m_is_initialized = true;
    return Result::Ok;
}

Result ALBackend::create_sound(std::filesystem::path const& file_path, SoundMode sound_mode, std::shared_ptr<ISound>& out)
{
    if (!std::filesystem::exists(file_path)) { return Result::ErrFileNotFound; }

    std::shared_ptr<IDecoder> decoder = std::make_shared<VorbisDecoder>();
    if (auto res = decoder->init(file_path); res != Result::Ok) { return res; }

    out = std::make_shared<ALSound>(decoder, sound_mode);
    return Result::Ok;
}

Result ALBackend::bind_sound(std::shared_ptr<ISound> const& sound, std::shared_ptr<IChannel>& out)
{
    out = std::make_shared<ALChannel>();
    return Result::Ok;
}

Result ALBackend::set_listener_position_3d(std::array<float, 3> const& position)
{
    return Result::Ok;
}

Result ALBackend::set_listener_velocity_3d(std::array<float, 3> const& velocity)
{
    return Result::Ok;
}

Result ALBackend::set_listener_orientation_3d(std::array<float, 3> const& orientation)
{
    return Result::Ok;
}

void ALBackend::update() {}
