#include "vorbis_decoder.h"

#include <fstream>
#include <vector>

#include "stb_vorbis.c"

using namespace storm::audio;

namespace
{

constexpr size_t BUFFER_SIZE = 2048;

}

struct VorbisDecoder::Impl {
    Impl(bool force_stereo)
        : is_initialized {false}
        , force_stereo {force_stereo}
        , stream {nullptr}
        , channels {0}
        , sample_rate {0}
        , offset {0}
    {
    }

    ~Impl()
    {
        if (is_initialized) { stb_vorbis_close(stream); }
    }

    Result init(std::filesystem::path const& file_path)
    {
        auto file = std::ifstream(file_path, std::ios::binary);
        if (!file) { return Result::ErrFileOpenFailed; }

        m_file_data = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        int err = 0;
        stream  = stb_vorbis_open_memory(
            reinterpret_cast<unsigned char const*>(m_file_data.data()), static_cast<int>(m_file_data.size()), &err, nullptr);
        if (stream == nullptr) { return Result::ErrFileFormatInvalid; }

        auto const info = stb_vorbis_get_info(stream);

        channels    = info.channels;
        sample_rate = info.sample_rate;
        format      = info.channels == 2 ? SoundFormat::Stereo16 : SoundFormat::Mono16;

        is_initialized = true;

        return Result::Ok;
    }

    Result get_pcm_data(std::vector<uint8_t>& data, bool read_until_end)
    {
        if (read_until_end) {
            data.clear();

            size_t               sample_count = 0;
            std::vector<uint8_t> buffer       = {};

            while ((sample_count = decode_part(buffer)) > 0) {
                size_t const bytes_count = sample_count * sizeof(short);
                if (buffer.size() != bytes_count) { buffer.resize(bytes_count); }  // Shrink buffer to actual size

                data.insert(data.end(), buffer.begin(), buffer.end());
                offset += bytes_count;
            }
        } else {
            auto sample_count = decode_part(data);

            size_t const bytes_count = sample_count * sizeof(short);
            if (data.size() != bytes_count) { data.resize(bytes_count); }  // Shrink buffer to actual size

            offset += bytes_count;
        }

        return Result::Ok;
    }

    Result seek_start()
    {
        if (offset > 0) {
            stb_vorbis_seek_start(stream);
            offset = 0;
        }

        return Result::Ok;
    }

    size_t decode_part(std::vector<uint8_t>& data)
    {
        data.resize(BUFFER_SIZE);
        std::memset(data.data(), 0, data.size());

        size_t const max_samples_count = data.size() / sizeof(short);

        size_t sample_count = 0;
        while (sample_count < data.size()) {
            int const converted = stb_vorbis_get_samples_short_interleaved(
                stream, channels, reinterpret_cast<short*>(data.data()) + sample_count, static_cast<int>(max_samples_count - sample_count));
            if (converted == 0) { break; }

            sample_count += converted * channels;
        }

        return sample_count;
    }

    bool is_initialized;

    bool force_stereo;  // FIXME: is it really needed here?

    stb_vorbis* stream;

    std::vector<char> m_file_data;

    int channels;
    int sample_rate;

    size_t offset;

    SoundFormat format;
};

VorbisDecoder::VorbisDecoder(bool force_stereo) : m_impl {std::make_unique<Impl>(force_stereo)} {}

VorbisDecoder::~VorbisDecoder() = default;

Result VorbisDecoder::init(std::filesystem::path const& file_path)
{
    return m_impl->init(file_path);
}

Result VorbisDecoder::get_channels(int& channels)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    channels = m_impl->channels;
    return Result::Ok;
}

Result VorbisDecoder::get_sample_rate(int& sample_rate)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    sample_rate = m_impl->sample_rate;
    return Result::Ok;
}

Result VorbisDecoder::get_sound_format(SoundFormat& format)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }

    format = m_impl->format;
    return Result::Ok;
}

Result VorbisDecoder::get_pcm_data(std::vector<uint8_t>& data, bool read_until_end)
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }
    return m_impl->get_pcm_data(data, read_until_end);
}

Result VorbisDecoder::seek_start()
{
    if (!m_impl->is_initialized) { return Result::ErrNotInitialized; }
    return m_impl->seek_start();
}
