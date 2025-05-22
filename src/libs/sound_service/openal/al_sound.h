#pragma once

#include <array>
#include <memory>

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALChannel;

class ALSound: virtual public ISound
{
public:
    ALSound(std::shared_ptr<IDecoder> const& decoder, ISound::Flags flags);
    ~ALSound() override;

    ISound::Flags get_flags() const override;

    int         get_channels() const override;
    int         get_sample_rate() const override;
    SoundFormat get_sound_format() const override;

private:
    friend class ALChannel;

    Result bind_buffers_to_source(unsigned source, bool is_looping);
    Result unbind_source(unsigned source);

    Result set_looping(unsigned source, bool is_looping);

    bool push_next_data(unsigned buffer, bool is_looping) const;
    void reset_buffers();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm::audio
