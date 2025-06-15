//============================================================================================
//    Spirenkov Maxim, 2003
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    Lizards
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "lizards.h"

#include <libs/core/core.h>
#include <libs/core/entity.h>

Lizards::Lizards()
{
    num = sizeof(lizard) / sizeof(Lizard);
}

Lizards::~Lizards() {}

// Initialization
bool Lizards::Init()
{
    // Location Pointer
    auto const loc      = core->GetEntityId("Location");
    auto*      location = (Location*)core->GetEntityPointer(loc);
    if (!location) return false;
    // init lizards
    for (int32_t i = 0; i < num; i++)
        lizard[i].Init(location);
    // Execution
    // core->LayerCreate("realize", true, false);
    core->SetLayerType(REALIZE, layer_type_t::realize);
    core->AddToLayer(REALIZE, GetId(), 100000);
    return true;
}

// Execution
void Lizards::Execute(uint32_t delta_time) {}

// Drawing
void Lizards::Realize(uint32_t delta_time)
{
    auto const dltTime = delta_time * 0.001f;
    for (int32_t i = 0; i < num; i++)
        lizard[i].Update(dltTime);
}
