#pragma once

#include <libs/geometry/geometry.h>
#include <libs/math/c_vector.h>
#include <libs/sea/sea_base.h>

#include "ball_splash_defines.h"
#include "t_splash.h"

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////

class BallSplash: public Entity
{
public:
    BallSplash();
    ~BallSplash() override;

    bool         Init() override;
    uint64_t     ProcessMessage(MESSAGE& message) override;
    virtual void Realize(uint32_t dTime);
    virtual void Execute(uint32_t dTime);

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
                LostRender(delta); break;
            case Stage::restore_render:
                RestoreRender(delta); break;*/
        }
    }

private:
    void     InitializeSplashes();
    TSplash* TryToAddSplash(const CVECTOR& _pos, const CVECTOR& _dir);

    TSplash splashes[MAX_SPLASHES];
    // FIXME: Renderer Next
    // VDX9RENDER* renderer;
    SEA_BASE* sea;
};
