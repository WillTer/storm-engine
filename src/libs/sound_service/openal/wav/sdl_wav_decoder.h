#pragma once

#include <memory>

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class SDLWavDecoder: virtual public IDecoder
{
public:
    SDLWavDecoder();
    ~SDLWavDecoder() override;

    std::unique_ptr<IDataStream> decode_file(std::filesystem::path const& file_path, IDataStream::Format output_format) override;
};

}  // namespace storm::audio
