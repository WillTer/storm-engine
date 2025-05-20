#include "sdl_wav_decoder.h"

#include <fstream>
#include <vector>

#include <SDL2/SDL_audio.h>

using namespace storm::audio;

struct SDLWavDecoder::Impl {
    Impl() : is_initialized {false}, channels {0}, sample_rate {0} {}

    ~Impl() = default;

    Result init(std::filesystem::path const& file_path)
    {
        auto file = std::ifstream(file_path, std::ios::binary);
        if (!file) { return Result::ErrFileOpenFailed; }

        auto const file_data = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        SDL_AudioSpec audio_spec = {};

        Uint8* pcm_buffer      = nullptr;
        Uint32 pcm_buffer_size = 0;
        if (SDL_LoadWAV_RW(
                SDL_RWFromConstMem(file_data.data(), static_cast<int>(file_data.size())),
                SDL_FALSE,
                &audio_spec,
                &pcm_buffer,
                &pcm_buffer_size)
            == nullptr) {
            return Result::ErrFileFormatInvalid;
        }

        pcm_data = std::vector<uint8_t>(pcm_buffer, pcm_buffer + pcm_buffer_size);

        channels    = audio_spec.channels;
        sample_rate = audio_spec.freq;

        bool const is_8bit  = (audio_spec.format & (AUDIO_S8 | AUDIO_U8)) != 0;
        bool const is_16bit = (audio_spec.format & (AUDIO_S16 | AUDIO_U16)) != 0;
        if (!is_8bit && !is_16bit) { return Result::ErrFileFormatNotSupported; }  // TODO: use SDL_AudioStream

        switch (audio_spec.channels) {
        case 1: format = is_8bit ? SoundFormat::Mono8 : SoundFormat::Mono16; break;
        case 2: format = is_8bit ? SoundFormat::Stereo8 : SoundFormat::Stereo16; break;
        default: return Result::ErrFileFormatNotSupported;
        }

        is_initialized = true;

        return Result::Ok;
    }

    Result get_pcm_data(std::vector<uint8_t>& data, bool read_until_end)
    {
        data.resize(0);
        // std::memset(data.data(), 0, data.size());

        return Result::Ok;
    }

    Result seek_start()
    {
        return Result::Ok;
    }

    bool is_initialized;

    std::vector<uint8_t> pcm_data;

    int channels;
    int sample_rate;

    SoundFormat format;
};

SDLWavDecoder::SDLWavDecoder() : m_impl {std::make_unique<Impl>()} {}

SDLWavDecoder::~SDLWavDecoder() = default;

Result SDLWavDecoder::init(std::filesystem::path const& file_path)
{
    return m_impl->init(file_path);
}

Result SDLWavDecoder::get_channels(int& channels)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    channels = m_impl->channels;
    return Result::Ok;
}

Result SDLWavDecoder::get_sample_rate(int& sample_rate)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    sample_rate = m_impl->sample_rate;
    return Result::Ok;
}

Result SDLWavDecoder::get_sound_format(SoundFormat& format)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    format = m_impl->format;
    return Result::Ok;
}

Result SDLWavDecoder::get_pcm_data(std::vector<uint8_t>& data, bool read_until_end)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->get_pcm_data(data, read_until_end);
}

Result SDLWavDecoder::seek_start()
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    return m_impl->seek_start();
}
