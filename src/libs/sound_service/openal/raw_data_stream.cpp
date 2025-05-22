#include "raw_data_stream.h"

#include <vector>

using namespace storm::audio;

struct RawDataStream::Impl {
    Impl(std::vector<uint8_t>&& data, int channels, int sample_rate, Format format)
        : m_data {std::move(data)}
        , m_channels {channels}
        , m_sample_rate {sample_rate}
        , m_format {format}
        , m_offset {0}
    {
    }

    ~Impl() = default;

    size_t get_pcm_data(std::vector<uint8_t>& buffer)
    {
        auto const remaining_bytes = m_data.size() - m_offset;
        if (remaining_bytes == 0) { return 0; }

        auto bytes_to_copy = std::min(buffer.size(), remaining_bytes);
        if (bytes_to_copy == 0) { bytes_to_copy = remaining_bytes; }
        if (buffer.size() != bytes_to_copy) { buffer.resize(bytes_to_copy); }

        std::memcpy(buffer.data(), m_data.data() + m_offset, buffer.size());
        m_offset += bytes_to_copy;

        return bytes_to_copy;
    }

    void seek_start()
    {
        m_offset = 0;
    }

    std::vector<uint8_t> m_data;
    int                  m_channels;
    int                  m_sample_rate;
    Format               m_format;

    size_t m_offset;
};

RawDataStream::RawDataStream(std::vector<uint8_t>&& data, int channels, int sample_rate, Format format)
    : m_impl {std::make_unique<Impl>(std::move(data), channels, sample_rate, format)}
{
}

RawDataStream::~RawDataStream() = default;

bool RawDataStream::is_valid()
{
    return true;
}

int RawDataStream::get_channels() const
{
    return m_impl->m_channels;
}

int RawDataStream::get_sample_rate() const
{
    return m_impl->m_sample_rate;
}

IDataStream::Format RawDataStream::get_data_format() const
{
    return m_impl->m_format;
}

size_t RawDataStream::get_pcm_data(std::vector<uint8_t>& buffer)
{
    return m_impl->get_pcm_data(buffer);
}

void RawDataStream::seek_start()
{
    m_impl->seek_start();
}
