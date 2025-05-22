#include "converted_data_stream.h"

#include <vector>

#include <AL/al.h>
#include <SDL2/SDL_audio.h>

#include "format_helpers.h"

using namespace storm::audio;

namespace
{
constexpr size_t INTERMEDIATE_BUFFER_DEFAULT_SIZE = 2048;
}

struct ConvertedDataStream::Impl {
    Impl(std::unique_ptr<IDataStream> stream, int output_channels, Format output_format)
        : m_is_initialized {false}
        , m_channels {output_channels}
        , m_format {output_format}
        , m_stream {std::move(stream)}
    {
        m_intermediate_buffer.reserve(INTERMEDIATE_BUFFER_DEFAULT_SIZE);

        m_sdl_stream = std::shared_ptr<SDL_AudioStream>(
            SDL_NewAudioStream(
                convert_to_sdl_format(m_stream->get_data_format()),
                m_stream->get_channels(),
                m_stream->get_sample_rate(),
                convert_to_sdl_format(m_format),
                m_channels,
                m_stream->get_sample_rate()),
            [](SDL_AudioStream* p) { SDL_FreeAudioStream(p); });
        if (!m_sdl_stream) { return; }

        m_is_initialized = true;
    }

    ~Impl() = default;

    size_t get_pcm_data(std::vector<uint8_t>& buffer)
    {
        m_intermediate_buffer.resize(buffer.size());
        std::memset(m_intermediate_buffer.data(), 0, m_intermediate_buffer.size());

        if (auto const read = m_stream->get_pcm_data(m_intermediate_buffer); read == 0) { return read; }
        SDL_AudioStreamPut(m_sdl_stream.get(), m_intermediate_buffer.data(), static_cast<int>(m_intermediate_buffer.size()));
        SDL_AudioStreamFlush(m_sdl_stream.get());

        buffer.resize(SDL_AudioStreamAvailable(m_sdl_stream.get()));
        std::memset(buffer.data(), 0, buffer.size());

        int const read = SDL_AudioStreamGet(m_sdl_stream.get(), buffer.data(), static_cast<int>(buffer.size()));
        if (buffer.size() != read) { buffer.resize(read); }

        return read;
    }

    bool   m_is_initialized;
    int    m_channels;
    Format m_format;

    std::unique_ptr<IDataStream>     m_stream;
    std::shared_ptr<SDL_AudioStream> m_sdl_stream;
    std::vector<uint8_t>             m_intermediate_buffer;
};

ConvertedDataStream::ConvertedDataStream(std::unique_ptr<IDataStream> stream, int output_channels, Format output_format)
    : m_impl {std::make_unique<Impl>(std::move(stream), output_channels, output_format)}
{
}

ConvertedDataStream::~ConvertedDataStream() = default;

bool ConvertedDataStream::is_valid()
{
    return m_impl->m_is_initialized;
}

int ConvertedDataStream::get_channels() const
{
    if (!m_impl->m_is_initialized) { return 0; }

    return m_impl->m_channels;
}

int ConvertedDataStream::get_sample_rate() const
{
    if (!m_impl->m_is_initialized) { return 0; }

    return m_impl->m_stream->get_sample_rate();
}

IDataStream::Format ConvertedDataStream::get_data_format() const
{
    if (!m_impl->m_is_initialized) { return Format::Unknown; }

    return m_impl->m_format;
}

size_t ConvertedDataStream::get_pcm_data(std::vector<uint8_t>& buffer)
{
    if (!m_impl->m_is_initialized) { return 0; }

    return m_impl->get_pcm_data(buffer);
}

void ConvertedDataStream::seek_start()
{
    if (!m_impl->m_is_initialized) { return; }

    m_impl->m_stream->seek_start();
}
