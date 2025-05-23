#include "sdl_wav_decoder.h"

#include <vector>

#include <SDL2/SDL_audio.h>
#include <libs/sound_service/openal/format_helpers.h>

using namespace storm::audio;

struct SDLWavDecoder::Impl {
    Impl() : m_is_valid {false}, m_channels {0}, m_sample_rate {0}, m_format {Format::Unknown}, m_sample_size {1}, m_offset {0} {}
    ~Impl() = default;

    bool load_memory(std::vector<uint8_t> const& mem)
    {
        SDL_AudioSpec audio_spec = {};

        Uint8* pcm_buffer      = nullptr;
        Uint32 pcm_buffer_size = 0;
        if (SDL_LoadWAV_RW(
                SDL_RWFromConstMem(mem.data(), static_cast<int>(mem.size())), SDL_FALSE, &audio_spec, &pcm_buffer, &pcm_buffer_size)
            == nullptr) {
            return false;
        }

        m_data        = std::vector<uint8_t>(pcm_buffer, pcm_buffer + pcm_buffer_size);
        m_channels    = audio_spec.channels;
        m_sample_rate = audio_spec.freq;
        m_format      = convert_from_sdl_format(audio_spec.format);
        m_sample_size = get_format_sample_size(m_format);

        SDL_FreeWAV(pcm_buffer);
        m_is_valid = true;

        return true;
    }

    size_t get_samples(std::vector<uint8_t>& buffer, size_t sample_count)
    {
        auto const bytes_count = m_data.size() - m_offset;
        if (bytes_count == 0 || sample_count == 0) { return 0; }

        auto const bytes_to_copy = std::min(sample_count * m_sample_size, bytes_count);
        buffer.resize(bytes_to_copy);

        std::memcpy(buffer.data(), m_data.data() + m_offset, buffer.size());
        m_offset += bytes_to_copy;

        return bytes_to_copy;
    }

    size_t get_samples_all(std::vector<uint8_t>& buffer)
    {
        auto const bytes_count = m_data.size() - m_offset;
        if (bytes_count == 0) { return 0; }

        return get_samples(buffer, bytes_count / m_sample_size);
    }

    void seek_start()
    {
        m_offset = 0;
    }

    bool m_is_valid;

    std::vector<uint8_t> m_data;
    int                  m_channels;
    int                  m_sample_rate;
    Format               m_format;

    size_t m_sample_size;
    size_t m_offset;
};

SDLWavDecoder::SDLWavDecoder() : m_impl {std::make_unique<Impl>()} {}

SDLWavDecoder::~SDLWavDecoder() = default;

bool SDLWavDecoder::load_memory(std::vector<uint8_t> const& mem)
{
    return m_impl->load_memory(mem);
}

bool SDLWavDecoder::is_valid()
{
    return m_impl->m_is_valid;
}

int SDLWavDecoder::get_channels() const
{
    return m_impl->m_channels;
}

int SDLWavDecoder::get_sample_rate() const
{
    return m_impl->m_sample_rate;
}

IDataStream::Format SDLWavDecoder::get_data_format() const
{
    return m_impl->m_format;
}

size_t SDLWavDecoder::get_samples(std::vector<uint8_t>& buffer, size_t sample_count)
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->get_samples(buffer, sample_count);
}

size_t SDLWavDecoder::get_samples_all(std::vector<uint8_t>& buffer)
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->get_samples_all(buffer);
}

void SDLWavDecoder::seek_start()
{
    m_impl->seek_start();
}
