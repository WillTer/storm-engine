#pragma once

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALSound: virtual public ISound
{
public:
    ALSound(std::shared_ptr<IDecoder> const& decoder, SoundMode sound_mode);
    ~ALSound() override;

private:
    std::shared_ptr<IDecoder> m_decoder;

    SoundMode m_sound_mode;
};

}  // namespace storm::audio
