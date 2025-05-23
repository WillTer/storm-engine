#include "converted_data_stream.h"

#include <AL/al.h>
#include <SDL2/SDL_audio.h>

#include "const.h"
#include "format_helpers.h"

using namespace storm::audio;

struct ConvertedDataStream::Impl {
    Impl(std::unique_ptr<IDataStream> stream, int output_channels, Format output_format)
        : m_is_valid {false}
        , m_channels {output_channels}
        , m_format {output_format}
        , m_unconverted_sample_size {0}
        , m_sample_size {0}
        , m_stream {std::move(stream)}
    {
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

        m_unconverted_sample_size = get_format_sample_size(m_stream->get_data_format());
        m_sample_size             = get_format_sample_size(m_format);

        m_unconverted_buffer.reserve(BUFFER_SAMPLE_COUNT * m_unconverted_sample_size);

        m_is_valid = true;
    }

    ~Impl() = default;

    size_t get_samples(std::vector<uint8_t>& buffer, size_t sample_count)
    {
        m_unconverted_buffer.resize(sample_count * m_unconverted_sample_size);

        if (auto const samples = m_stream->get_samples(m_unconverted_buffer, sample_count); samples == 0) { return samples; }
        convert_buffer(m_unconverted_buffer);

        buffer.resize(SDL_AudioStreamAvailable(m_sdl_stream.get()));
        return SDL_AudioStreamGet(m_sdl_stream.get(), buffer.data(), static_cast<int>(buffer.size()));
    }

    size_t get_samples_all(std::vector<uint8_t>& buffer)
    {
        std::vector<uint8_t> unconverted_buffer = {};

        if (auto const samples = m_stream->get_samples_all(unconverted_buffer); samples == 0) { return samples; }
        convert_buffer(unconverted_buffer);

        buffer.resize(SDL_AudioStreamAvailable(m_sdl_stream.get()));
        return SDL_AudioStreamGet(m_sdl_stream.get(), buffer.data(), static_cast<int>(buffer.size()));
    }

    void convert_buffer(std::vector<uint8_t> const& buffer)
    {
        SDL_AudioStreamPut(m_sdl_stream.get(), buffer.data(), static_cast<int>(buffer.size()));
        SDL_AudioStreamFlush(m_sdl_stream.get());
    }

    bool   m_is_valid;
    int    m_channels;
    Format m_format;

    size_t m_unconverted_sample_size;
    size_t m_sample_size;

    std::unique_ptr<IDataStream>     m_stream;
    std::shared_ptr<SDL_AudioStream> m_sdl_stream;
    std::vector<uint8_t>             m_unconverted_buffer;
};

ConvertedDataStream::ConvertedDataStream(std::unique_ptr<IDataStream> stream, int output_channels, Format output_format)
    : m_impl {std::make_unique<Impl>(std::move(stream), output_channels, output_format)}
{
}

ConvertedDataStream::~ConvertedDataStream() = default;

bool ConvertedDataStream::load_file(std::filesystem::path const& /*file_path*/)
{
    return false;
}

bool ConvertedDataStream::load_memory(std::vector<uint8_t> const& /*mem*/)
{
    return false;
}

bool ConvertedDataStream::is_valid()
{
    return m_impl->m_is_valid;
}

int ConvertedDataStream::get_channels() const
{
    return m_impl->m_channels;
}

int ConvertedDataStream::get_sample_rate() const
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->m_stream->get_sample_rate();
}

IDataStream::Format ConvertedDataStream::get_data_format() const
{
    return m_impl->m_format;
}

size_t ConvertedDataStream::get_samples(std::vector<uint8_t>& buffer, size_t sample_count)
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->get_samples(buffer, sample_count);
}

size_t ConvertedDataStream::get_samples_all(std::vector<uint8_t>& buffer)
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->get_samples_all(buffer);
}

void ConvertedDataStream::seek_start()
{
    if (!m_impl->m_is_valid) { return; }

    m_impl->m_stream->seek_start();
}
