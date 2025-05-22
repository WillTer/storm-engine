#include "vorbis_decoder.h"

#include <fstream>
#include <functional>
#include <vector>

#include <libs/sound_service/openal/format_helpers.h>
#include <libs/sound_service/openal/raw_data_stream.h>

#include "stb_vorbis.c"

using namespace storm::audio;

namespace
{
constexpr size_t BUFFER_SIZE = 2048;

size_t get_samples_short(stb_vorbis* const& stream, int channels, void* buffer, size_t buffer_size, size_t offset)
{
    return stb_vorbis_get_samples_short_interleaved(
        stream, channels, reinterpret_cast<short*>(buffer) + offset, static_cast<int>(buffer_size - offset));
}

size_t get_samples_float(stb_vorbis* const& stream, int channels, void* buffer, size_t buffer_size, size_t offset)
{
    return stb_vorbis_get_samples_float_interleaved(
        stream, channels, reinterpret_cast<float*>(buffer) + offset, static_cast<int>(buffer_size - offset));
}

size_t
decode_part(stb_vorbis* stream, int const channels, size_t const sample_size, auto get_samples_callback, std::vector<uint8_t>& buffer)
{
    assert(buffer.size() % sample_size == 0);
    std::memset(buffer.data(), 0, buffer.size());

    size_t const max_samples_count = buffer.size() / sample_size;

    size_t sample_count = 0;
    while (sample_count < buffer.size()) {
        auto const converted = get_samples_callback(stream, channels, buffer.data(), max_samples_count, sample_count);
        if (converted == 0) { break; }

        sample_count += converted * channels;
    }

    size_t const bytes_count = sample_count * sample_size;
    if (buffer.size() != bytes_count) { buffer.resize(bytes_count); }  // Shrink buffer to actual size

    return bytes_count;
}

std::vector<uint8_t> get_pcm_data(stb_vorbis* stream, int const channels, size_t const sample_size, auto get_samples_callback)
{
    std::vector<uint8_t> out_buffer = {};
    std::vector<uint8_t> buffer     = {};
    buffer.resize(BUFFER_SIZE);

    // Read everything
    while (decode_part(stream, channels, sample_size, get_samples_callback, buffer) > 0) {
        out_buffer.insert(out_buffer.end(), buffer.begin(), buffer.end());
    }

    return out_buffer;
}

}  // namespace

VorbisDecoder::VorbisDecoder() = default;

VorbisDecoder::~VorbisDecoder() = default;

std::unique_ptr<IDataStream> VorbisDecoder::decode_file(std::filesystem::path const& file_path, IDataStream::Format output_format)
{
    auto file = std::ifstream(file_path, std::ios::binary);
    if (!file) { return nullptr; }

    auto file_data = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    int  err    = 0;
    auto stream = std::shared_ptr<stb_vorbis>(
        stb_vorbis_open_memory(reinterpret_cast<unsigned char const*>(file_data.data()), static_cast<int>(file_data.size()), &err, nullptr),
        [](stb_vorbis* p) { stb_vorbis_close(p); });
    if (!stream) { return nullptr; }

    auto const info = stb_vorbis_get_info(stream.get());

    auto const get_samples_callback = output_format == IDataStream::Format::Float32 ? get_samples_float : get_samples_short;
    auto const sample_size          = output_format == IDataStream::Format::Float32 ? sizeof(float) : sizeof(short);

    auto pcm_data = get_pcm_data(stream.get(), info.channels, sample_size, get_samples_callback);

    return std::make_unique<RawDataStream>(std::move(pcm_data), info.channels, info.sample_rate, output_format);
}
