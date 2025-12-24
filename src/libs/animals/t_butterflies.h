#pragma once

#include <libs/animation/animation.h>
#include <libs/math/matrix.h>

#include "animals_defines.h"
#include "t_butterfly.h"

#define Y_REDEFINE_TIME 250

class COLLIDE;

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////
class TButterflies
{
public:
    TButterflies();
    virtual ~TButterflies();

    uint64_t ProcessMessage(int32_t _code, MESSAGE& message);
    void     Init();
    void     Realize(uint32_t dTime);
    void     Execute(uint32_t dTime);

private:
    void LoadSettings();

    // FIXME: Renderer Next
    // VDX9RENDER*      renderService;
    // IVBufferManager* ivManager;
    COLLIDE*   collide;
    entid_t    butterflyModel;
    TButterfly butterflies[BUTTERFLY_COUNT];
    int32_t    butterfliesCount;

    float maxDistance;

    bool    enabled;
    int32_t yDefineTime;
    int32_t texture;
};
