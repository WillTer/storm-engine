#pragma once

#include <libs/geometry/geometry.h>
#include <libs/math/c_vector.h>
#include <libs/renderer/dx9render.h>
#include <libs/sea/sea_base.h>

#include "sink_splash_defines.h"
#include "t_sink.h"

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////

class SinkEffect: public Entity
{
public:
    SinkEffect();
    ~SinkEffect() override;

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
    void   InitializeSinks();
    TSink* TryToAddSink(const CVECTOR& _pos, float _r);

    TSink       sinks[sink_effect::MAX_SINKS];
    VDX9RENDER* renderer;
    SEA_BASE*   sea;
};
