//============================================================================================
//    Spirenkov Maxim, 2003
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    LocRats
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "loc_rats.h"

#include <libs/core/core.h>
#include <libs/core/entity.h>

//============================================================================================

LocRats::LocRats()
{
    num = sizeof(rat) / sizeof(LocRat);
}

LocRats::~LocRats() {}

//============================================================================================

// Initialization
bool LocRats::Init(std::shared_ptr<storm::ServiceLocator> const& service_locator)
{
    Entity::Init(service_locator);

    // Location Pointer
    auto const loc      = core.GetEntityId("location");
    auto*      location = (Location*)core.GetEntityPointer(loc);
    if (!location) return false;
    // Execution
    // core.LayerCreate("realize", true, false);
    core.SetLayerType(REALIZE, layer_type_t::realize);
    core.AddToLayer(REALIZE, GetId(), 100000);
    return true;
}

// Messages
uint64_t LocRats::ProcessMessage(MESSAGE& message)
{
    auto num = message.Long();
    if (num < 1) num = 1;
    if (num > sizeof(rat) / sizeof(LocRat)) num = sizeof(rat) / sizeof(LocRat);
    // Location Pointer
    auto const loc      = core.GetEntityId("location");
    auto*      location = (Location*)core.GetEntityPointer(loc);
    if (!location) return 0;
    // Init rats
    for (int32_t i = 0; i < num; i++)
        rat[i].Init(location);
    return 1;
}

// Execution
void LocRats::Execute(uint32_t delta_time) {}

// Drawing
void LocRats::Realize(uint32_t delta_time)
{
    auto const dltTime = delta_time * 0.001f;
    for (int32_t i = 0; i < num; i++)
        rat[i].Update(dltTime);
}
