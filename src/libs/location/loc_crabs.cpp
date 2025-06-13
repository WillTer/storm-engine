//============================================================================================
//    LocCrabs
//============================================================================================

#include "loc_crabs.h"

#include <libs/core/core.h>
#include <libs/core/entity.h>

//============================================================================================

LocCrabs::LocCrabs()
{
    num = sizeof(crab) / sizeof(LocCrab);
}

LocCrabs::~LocCrabs() {}

//============================================================================================

// Initialization
bool LocCrabs::Init(std::shared_ptr<storm::ServiceLocator> const& service_locator)
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
uint64_t LocCrabs::ProcessMessage(MESSAGE& message)
{
    int32_t num = message.Long();
    if (num < 1) num = 1;
    if (num > sizeof(crab) / sizeof(LocCrab)) num = sizeof(crab) / sizeof(LocCrab);
    // Location Pointer
    auto const loc      = core.GetEntityId("location");
    auto*      location = (Location*)core.GetEntityPointer(loc);
    if (!location) return 0;
    // start crabs
    for (int32_t i = 0; i < num; i++)
        crab[i].Init(location);
    return 1;
}

// Execution
void LocCrabs::Execute(uint32_t delta_time) {}

// Drawing
void LocCrabs::Realize(uint32_t delta_time)
{
    float dltTime = delta_time * 0.001f;
    for (int32_t i = 0; i < num; i++)
        crab[i].Update(dltTime);
}
