#pragma once

// common includes
#include <entt/fwd.hpp>

#include "core.h"
#include "service.h"

class CorePrivate: public Core
{
public:
    virtual void Init(std::shared_ptr<entt::registry> const& registry) = 0;

    virtual void InitBase()    = 0;
    virtual void ReleaseBase() = 0;

    virtual void CleanUp() = 0;

    virtual bool               Run()               = 0;
    [[nodiscard]] virtual bool initialized() const = 0;

    virtual void AppState(bool state) = 0;

    virtual void collectCrashInfo() const = 0;

    virtual void SetWindow(std::shared_ptr<storm::OSWindow> window) = 0;

    virtual void set_organizer_for_section_start(uint32_t section, entt::organizer& organizer) = 0;
    virtual void set_organizer_for_section_end(uint32_t section, entt::organizer& organizer)   = 0;
};
