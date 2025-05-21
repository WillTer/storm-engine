#pragma once

#include <memory>

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class SDLWavDecoder: virtual public IDecoder
{
public:
    SDLWavDecoder(bool force_stereo);
    ~SDLWavDecoder() override;

    Result init(std::filesystem::path const& file_path) override;

    Result get_channels(int& channels) override;
    Result get_sample_rate(int& sample_rate) override;

    Result get_sound_format(SoundFormat& format) override;

    Result get_pcm_data(std::vector<uint8_t>& data, bool read_until_end) override;
    Result seek_start() override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm::audio
