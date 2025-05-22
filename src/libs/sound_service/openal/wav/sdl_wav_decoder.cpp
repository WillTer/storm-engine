#include "sdl_wav_decoder.h"

#include <fstream>
#include <vector>

#include <SDL2/SDL_audio.h>
#include <libs/sound_service/openal/format_helpers.h>
#include <libs/sound_service/openal/raw_data_stream.h>

using namespace storm::audio;

SDLWavDecoder::SDLWavDecoder() = default;

SDLWavDecoder::~SDLWavDecoder() = default;

std::unique_ptr<IDataStream> SDLWavDecoder::decode_file(std::filesystem::path const& file_path, IDataStream::Format /*output_format*/)
{
    auto file = std::ifstream(file_path, std::ios::binary);
    if (!file) { return nullptr; }

    auto const file_data = std::vector((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    SDL_AudioSpec audio_spec = {};

    Uint8* pcm_buffer      = nullptr;
    Uint32 pcm_buffer_size = 0;
    if (SDL_LoadWAV_RW(
            SDL_RWFromConstMem(file_data.data(), static_cast<int>(file_data.size())), SDL_FALSE, &audio_spec, &pcm_buffer, &pcm_buffer_size)
        == nullptr) {
        return nullptr;
    }

    auto pcm_data = std::vector<uint8_t>(pcm_buffer, pcm_buffer + pcm_buffer_size);

    return std::make_unique<RawDataStream>(
        std::move(pcm_data), audio_spec.channels, audio_spec.freq, convert_from_sdl_format(audio_spec.format));
}
