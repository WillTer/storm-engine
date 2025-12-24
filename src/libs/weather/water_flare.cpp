#include "water_flare.h"

#include <libs/core/core.h>
#include <libs/math/math_inlines.h>
#include <stdio.h>

#define WATERFLARE_DIR "weather/sea/flare/"

WaterFlare::WaterFlare()
{
    iFlareTex  = -1;
    iFlaresNum = 0;
    pWeather   = nullptr;
    pfAlpha    = nullptr;
}

WaterFlare::~WaterFlare()
{
    STORM_DELETE(pfAlpha);
}

bool WaterFlare::Init()
{
    core->AddToLayer(REALIZE, GetId(), -1);
    core->AddToLayer(EXECUTE, GetId(), -1);

    SetDevice();
    return true;
}

void WaterFlare::SetDevice()
{
    entid_t ent;
    if (!(ent = core->GetEntityId("Weather"))) throw std::runtime_error("No found WEATHER entity!");
    pWeather = static_cast<WEATHER_BASE*>(core->GetEntityPointer(ent));
}

bool WaterFlare::CreateState(ENTITY_STATE_GEN* state_gen)
{
    return true;
}

bool WaterFlare::LoadState(ENTITY_STATE* state)
{
    return true;
}

void WaterFlare::Execute(uint32_t Delta_Time) {}

void WaterFlare::GenerateFlares()
{
    // iFlaresNum = 1024 + (rand() % 64);
    // pRSRect    = static_cast<RS_RECT*>(new RS_RECT[iFlaresNum]);
    // pfAlpha    = static_cast<float*>(new float[iFlaresNum]);
    // for (int32_t i = 0; i < iFlaresNum; i++) {
    //     pfAlpha[i]              = FRAND(-40.0f);
    //     pRSRect[i].vPos         = CVECTOR(FRAND(1000.0f), 0.0f, FRAND(1000.0f));
    //     pRSRect[i].fAngle       = 0.0f;
    //     pRSRect[i].dwSubTexture = 0;
    //     pRSRect[i].fSize        = 0.5f;
    // }
}

void WaterFlare::Realize(uint32_t Delta_Time) const
{
    // for (int32_t i = 0; i < iFlaresNum; i++) {
    //     auto const fDeltaTime = static_cast<float>(Delta_Time) * 0.001f;
    //     pfAlpha[i] += fDeltaTime;
    //     if (pfAlpha[i] > 2.0f) {
    //         pfAlpha[i]      = 0.0f;
    //         pRSRect[i].vPos = CVECTOR(FRAND(200.0f), 0.0f, FRAND(200.0f));
    //     }
    //     auto const dwAlpha = static_cast<uint32_t>(255.0f * ((pfAlpha[i] > 1.0f) ? 2.0f - pfAlpha[i] : pfAlpha[i]));
    //     pRSRect[i].dwColor = makeRGB(dwAlpha, dwAlpha, dwAlpha);
    // }
    //
    // RS->TextureSet(0, iFlareTex);
    // RS->DrawRects(pRSRect, iFlaresNum, "WaterFlare");
}

void WaterFlare::ProcessMessage(uint32_t iMsg, uint32_t wParam, uint32_t lParam) {}
