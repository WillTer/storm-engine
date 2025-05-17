#include "debug_entity.h"

#include <libs/core/core.h>
#include <libs/core/entity.h>
#include <libs/shared_headers/layers.h>

#include "sound_service.h"

SoundVisualisationEntity::SoundVisualisationEntity()
{
    pSound = nullptr;
}

SoundVisualisationEntity::~SoundVisualisationEntity()
{
}

bool SoundVisualisationEntity::Init()
{
    return true;
}

void SoundVisualisationEntity::Realize(uint32_t dTime)
{
    if (!pSound)
        return;
    pSound->DebugDraw();
}

void SoundVisualisationEntity::Execute(uint32_t dTime)
{
}

void SoundVisualisationEntity::SetMasterSoundService(SoundService *pSoundService)
{
    pSound = pSoundService;
}

void SoundVisualisationEntity::Wakeup() const
{
    // core.LayerCreate("sound_debug_realize", true , false);
    core.SetLayerType(SOUND_DEBUG_REALIZE, layer_type_t::realize);
    core.AddToLayer(SOUND_DEBUG_REALIZE, GetId(), -1);
}
