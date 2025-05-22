#pragma once

#include <memory>

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class RawDataStream: virtual public IDataStream
{
public:
    RawDataStream(std::vector<uint8_t>&& data, int channels, int sample_rate, Format format);
    ~RawDataStream() override;

    bool is_valid() override;

    int get_channels() const override;
    int get_sample_rate() const override;

    Format get_data_format() const override;

    size_t get_pcm_data(std::vector<uint8_t>& buffer) override;
    void   seek_start() override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm::audio
