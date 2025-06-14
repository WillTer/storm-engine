#pragma once

#include <entt/fwd.hpp>
#include <libs/animation/animation.h>
#include <libs/collide/collide.h>
#include <libs/geometry/geometry.h>
#include <libs/math/matrix.h>
#include <libs/renderer/dx9render.h>
#include <libs/renderer/iv_buffer_manager.h>

#include "animals_defines.h"
#include "t_butterfly.h"

#define Y_REDEFINE_TIME 250

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////
class TButterflies
{
public:
    TButterflies();
    virtual ~TButterflies();

    uint64_t ProcessMessage(int32_t _code, MESSAGE& message);
    void     Init(std::shared_ptr<entt::registry> const& registry);
    void     Realize(uint32_t dTime);
    void     Execute(uint32_t dTime);

private:
    void LoadSettings();

    std::shared_ptr<entt::registry> m_registry;

    VDX9RENDER*      renderService;
    IVBufferManager* ivManager;
    entid_t          butterflyModel;
    TButterfly       butterflies[BUTTERFLY_COUNT];
    int32_t          butterfliesCount;

    float maxDistance;

    bool    enabled;
    int32_t yDefineTime;
    int32_t texture;
};
