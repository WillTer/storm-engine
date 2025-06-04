#pragma once
#include <libs/math/c_vector.h>
#include <libs/renderer/dx9render.h>
#include <libs/renderer/iv_buffer_manager.h>
#include <libs/sea/sea_base.h>

#include "t_flotsam.h"
#include "t_sink_splash.h"

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////
class TSink
{
public:
    TSink();
    virtual ~TSink();

    void Initialize(INIFILE* _ini, IDirect3DDevice9* _device, SEA_BASE* sea, VDX9RENDER* _renderer);
    void Release();
    void Start(const CVECTOR& _pos, float _radius);
    void Process(uint32_t dTime);
    void Realize(uint32_t dTime);
    bool Enabled();

private:
    bool enabled;

    VDX9RENDER*      renderer;
    SEA_BASE*        sea;
    int32_t          texture;
    IVBufferManager* ivManager;
    uint32_t         time;

    TSinkSplash splashes[sink_effect::MAX_SPLASHES];
    int32_t     times[sink_effect::MAX_SPLASHES];
    int32_t     ivIndexes[sink_effect::MAX_SPLASHES];
    int32_t     flotsamTimes[sink_effect::MAX_FLOTSAMS];
    TFlotsam    flotsams[sink_effect::MAX_FLOTSAMS];
    float       distortDivider;
    float       radius;
    CVECTOR     center;
};
