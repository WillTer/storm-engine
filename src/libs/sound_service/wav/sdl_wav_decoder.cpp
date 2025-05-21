#include "sdl_wav_decoder.h"

#include <fstream>
#include <vector>

#include <SDL2/SDL_audio.h>

using namespace storm::audio;

namespace
{

constexpr size_t BUFFER_SIZE = 2048;

}

struct SDLWavDecoder::Impl {
    Impl(bool force_stereo) : is_initialized {false}, force_stereo {force_stereo}, channels {0}, sample_rate {0}, offset {0} {}

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

        channels    = force_stereo ? 2 : audio_spec.channels;
        sample_rate = audio_spec.freq;

        stream = SDL_NewAudioStream(audio_spec.format, audio_spec.channels, audio_spec.freq, audio_spec.format, channels, audio_spec.freq);
        seek_start();

        bool const is_8bit  = (audio_spec.format & AUDIO_S8) == AUDIO_S8 || (audio_spec.format & AUDIO_U8) == AUDIO_U8;
        bool const is_16bit = (audio_spec.format & AUDIO_S16) == AUDIO_S16 || (audio_spec.format & AUDIO_U16) == AUDIO_U16;
        if (!is_8bit && !is_16bit) { return Result::ErrFileFormatNotSupported; }  // TODO: use SDL_AudioStream
        switch (channels) {
        case 1: format = is_8bit ? SoundFormat::Mono8 : SoundFormat::Mono16; break;
        case 2: format = is_8bit ? SoundFormat::Stereo8 : SoundFormat::Stereo16; break;
        default: return Result::ErrFileFormatNotSupported;
        }

        is_initialized = true;

        return Result::Ok;
    }

    Result get_pcm_data(std::vector<uint8_t>& data, bool read_until_end)
    {
        size_t buffer_size = SDL_AudioStreamAvailable(stream);
        if (!read_until_end) { buffer_size = std::min(BUFFER_SIZE, buffer_size); }

        data.resize(buffer_size);
        SDL_AudioStreamGet(stream, data.data(), static_cast<int>(data.size()));

        offset += buffer_size;

        return Result::Ok;
    }

    Result seek_start()
    {
        offset = 0;

        SDL_AudioStreamClear(stream);
        SDL_AudioStreamPut(stream, pcm_data.data(), static_cast<int>(pcm_data.size()));
        SDL_AudioStreamFlush(stream);

        return Result::Ok;
    }

    bool is_initialized;

    bool force_stereo;

    SDL_AudioStream*     stream;
    std::vector<uint8_t> pcm_data;

    int channels;
    int sample_rate;

    size_t offset;

    SoundFormat format;
};

SDLWavDecoder::SDLWavDecoder(bool force_stereo) : m_impl {std::make_unique<Impl>(force_stereo)} {}

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
