#pragma once

#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

class ALSound: virtual public ISound
{
public:
    ALSound();
    ~ALSound() override;
};

}  // namespace storm::audio
