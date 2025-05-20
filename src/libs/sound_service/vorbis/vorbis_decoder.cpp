#include "vorbis_decoder.h"

#include <fstream>

#include "stb_vorbis.c"

using namespace storm::audio;

struct VorbisDecoder::Impl {
    Impl() : is_initialized {false}, stream {nullptr}, channels {0}, sample_rate {0} {}

    ~Impl()
    {
        if (is_initialized) { stb_vorbis_close(stream); }
    }

    Result init(std::filesystem::path const& file_path)
    {
        auto file = std::ifstream(file_path, std::ios::binary);
        if (!file) { return Result::ErrFileOpenFailed; }

        auto const file_data = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        int err = 0;
        stream  = stb_vorbis_open_memory(
            reinterpret_cast<unsigned char const*>(file_data.data()), static_cast<int>(file_data.size()), &err, nullptr);
        if (stream == nullptr) { return Result::ErrDecoderNotSupported; }

        auto const info = stb_vorbis_get_info(stream);

        channels    = info.channels;
        sample_rate = info.sample_rate;

        is_initialized = true;

        return Result::Ok;
    }

    bool is_initialized;

    stb_vorbis* stream;

    int channels;
    int sample_rate;
};

VorbisDecoder::VorbisDecoder() : m_impl {std::make_unique<Impl>()} {}

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
