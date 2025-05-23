#pragma once

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class BaseDataStream: virtual public IDataStream
{
public:
    BaseDataStream();
    ~BaseDataStream() override;

    bool load_file(std::filesystem::path const& file_path) override;
};

}  // namespace storm::audio
