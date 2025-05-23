#include "vorbis_decoder.h"

#include <functional>
#include <vector>

#include <libs/sound_service/openal/const.h>
#include <libs/sound_service/openal/format_helpers.h>

#include "stb_vorbis.c"

using namespace storm::audio;

namespace
{

size_t get_samples_short(stb_vorbis* stream, int channels, void* buffer, size_t buffer_size, size_t offset)
{
    return stb_vorbis_get_samples_short_interleaved(
        stream, channels, reinterpret_cast<short*>(buffer) + offset, static_cast<int>(buffer_size - offset));
}

size_t get_samples_float(stb_vorbis* stream, int channels, void* buffer, size_t buffer_size, size_t offset)
{
    return stb_vorbis_get_samples_float_interleaved(
        stream, channels, reinterpret_cast<float*>(buffer) + offset, static_cast<int>(buffer_size - offset));
}

}  // namespace

struct VorbisDecoder::Impl {
    Impl(IDataStream::Format output_format)
        : m_is_valid {false}
        , m_channels {0}
        , m_sample_rate {0}
        , m_format {output_format}
        , m_sample_size {0}
        , m_offset {0}
    {
    }

    ~Impl() = default;

    bool load_memory(std::vector<uint8_t> const& mem)
    {
        m_file_data = mem;

        int err  = 0;
        m_stream = std::shared_ptr<stb_vorbis>(
            stb_vorbis_open_memory(m_file_data.data(), static_cast<int>(m_file_data.size()), &err, nullptr),
            [](stb_vorbis* p) { stb_vorbis_close(p); });
        if (!m_stream) { return false; }

        auto const info = stb_vorbis_get_info(m_stream.get());

        m_channels    = info.channels;
        m_sample_rate = info.sample_rate;

        if (m_format == IDataStream::Format::Float32) {
            m_get_samples = get_samples_float;
            m_sample_size = sizeof(float);
        } else {
            m_get_samples = get_samples_short;
            m_sample_size = sizeof(short);
        }

        m_is_valid = true;
        return true;
    }

    size_t get_samples(std::vector<uint8_t>& buffer, size_t sample_count)
    {
        buffer.resize(sample_count * m_sample_size);

        size_t samples_read = 0;
        while (samples_read < buffer.size()) {
            auto const converted = m_get_samples(m_stream.get(), m_channels, buffer.data(), sample_count, samples_read);
            if (converted == 0) { break; }

            samples_read += converted * m_channels;
        }

        size_t const bytes_read = samples_read * m_sample_size;
        if (buffer.size() != bytes_read) { buffer.resize(bytes_read); }  // Shrink buffer to actual size

        m_offset += bytes_read;

        return samples_read;
    }

    size_t get_samples_all(std::vector<uint8_t>& out_buffer)
    {
        std::vector<uint8_t> buffer = {};

        size_t samples_read = 0;
        size_t read         = 0;

        // Read everything
        while ((read = get_samples(buffer, BUFFER_SAMPLE_COUNT)) > 0) {
            out_buffer.insert(out_buffer.end(), buffer.begin(), buffer.end());
            samples_read += read;
        }

        m_offset += samples_read * m_sample_rate;

        return samples_read;
    }

    void seek_start()
    {
        if (m_offset > 0) {
            stb_vorbis_seek_start(m_stream.get());
            m_offset = 0;
        }
    }

    bool m_is_valid;

    int m_channels;
    int m_sample_rate;

    IDataStream::Format m_format;
    size_t              m_sample_size;

    std::function<size_t(stb_vorbis*, int, void*, size_t, size_t)> m_get_samples;

    std::vector<uint8_t>        m_file_data;  //  stb_vorbis do not copy it, so we need to keep it alive
    std::shared_ptr<stb_vorbis> m_stream;
    size_t                      m_offset;
};

VorbisDecoder::VorbisDecoder(IDataStream::Format output_format) : m_impl {std::make_unique<Impl>(output_format)} {}

VorbisDecoder::~VorbisDecoder() = default;

bool VorbisDecoder::load_memory(std::vector<uint8_t> const& mem)
{
    return m_impl->load_memory(mem);
}

bool VorbisDecoder::is_valid()
{
    return m_impl->m_is_valid;
}

int VorbisDecoder::get_channels() const
{
    return m_impl->m_channels;
}

int VorbisDecoder::get_sample_rate() const
{
    return m_impl->m_sample_rate;
}

IDataStream::Format VorbisDecoder::get_data_format() const
{
    return m_impl->m_format;
}

size_t VorbisDecoder::get_samples(std::vector<uint8_t>& buffer, size_t sample_count)
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->get_samples(buffer, sample_count);
}

size_t VorbisDecoder::get_samples_all(std::vector<uint8_t>& buffer)
{
    if (!m_impl->m_is_valid) { return 0; }

    return m_impl->get_samples_all(buffer);
}

void VorbisDecoder::seek_start()
{
    m_impl->seek_start();
}
