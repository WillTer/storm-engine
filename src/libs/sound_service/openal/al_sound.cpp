#include "al_sound.h"

using namespace storm::audio;

ALSound::ALSound(std::shared_ptr<IDecoder> const& decoder, SoundMode sound_mode) : m_decoder {decoder}, m_sound_mode {sound_mode} {}

ALSound::~ALSound() = default;
