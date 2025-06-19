//============================================================================================
//    Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    WdmStormCloud
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "wdm_storm_cloud.h"

#include "wdm_objects.h"

//============================================================================================

#define WDM_STORMCLOUD_COLOR 0xff505055

//============================================================================================

WdmStormCloud::RainVertex WdmStormCloud::rain[4096];

// ============================================================================================
// Construction, destruction
// ============================================================================================

WdmStormCloud::WdmStormCloud()
{
    FillRects();
    constAlpha        = 1.0f;
    lightningWaitTime = 0.0f;
    curLightning      = -1;
    rainTexture       = -1;
    curU = curV = 0.0f;
    rainTexture = -1;  // wdmObjects->rs->TextureCreate("worldmap/rain.tga");
}

WdmStormCloud::~WdmStormCloud()
{
    // if (rainTexture >= 0) wdmObjects->rs->TextureRelease(rainTexture);
    rainTexture = -1;
}

void WdmStormCloud::BuildCloud(int32_t n)
{
    // Create a cloud
    WdmCloud::BuildCloud(n);
    // Filling in the colors
    FillRects();
}

void WdmStormCloud::FillRects()
{
    for (int32_t i = 0; i < numRects; i++) {
        // Colour
        float r = (WDM_STORMCLOUD_COLOR >> 16) & 0xff;
        float g = (WDM_STORMCLOUD_COLOR >> 8) & 0xff;
        float b = (WDM_STORMCLOUD_COLOR >> 0) & 0xff;
        r *= (0.9f + rand() * 0.1f / RAND_MAX);
        g *= (0.9f + rand() * 0.1f / RAND_MAX);
        b *= (0.9f + rand() * 0.1f / RAND_MAX);
        if (r < 0.0f) r = 0.0f;
        if (r > 255.0f) r = 255.0f;
        if (g < 0.0f) g = 0.0f;
        if (g > 255.0f) g = 255.0f;
        if (b < 0.0f) b = 0.0f;
        if (b > 255.0f) b = 255.0f;
        // rect[i].dwColor = 0xff000000 | (int32_t(r) << 16) | (int32_t(g) << 8) | (int32_t(b) << 0);
        // Rain coordinates
        for (int32_t j = 0; j < 256; j++) {
            float ang = rand() * 2.0f * 3.141592653f / RAND_MAX;
            // float r                = rand() * rect[i].fSize / RAND_MAX;
            rainpos[i * 256 + j].x = r * sinf(ang);
            rainpos[i * 256 + j].y = 0.0f;
            rainpos[i * 256 + j].z = r * cosf(ang);
        }
    }
}

// Calculations
void WdmStormCloud::Update(float dltTime)
{
    WdmCloud::Update(dltTime);
    // generate lightning
    if (curLightning < 0) {
        lightningWaitTime += dltTime * 0.001f;
        if (lightningWaitTime > 0.001f + rand() * 1.0f / RAND_MAX) {
            // time to emit a new lightning
            curLightning  = rand() % numRects;
            lightningTime = 0.3f;
            // lastColor      = rect[curLightning].dwColor;
            lightningColor = 0xa0cfff | (lastColor & 0xff000000);
            // rect[curLightning].dwColor = lightningColor;
            flashTime = 0.0f;
        }
    }
    if (curLightning >= 0) {
        flashTime += dltTime;
        if (flashTime > 0.05f) {
            // rect[curLightning].dwColor = (rand() & 1) ? lightningColor : lastColor;
            flashTime = 0.0f;
        }
        lightningTime -= dltTime;
        if (lightningTime < 0.0f) {
            // rect[curLightning].dwColor = lastColor;
            lightningWaitTime = 0.0f;
            curLightning      = -1;
        }
    }
    // Rain
    curU += 1.0f * dltTime;
    curV += 0.1f * dltTime;
    if (curU > 1.0f) curU -= 1.0f;
    if (curV > 1.0f) curV -= 1.0f;
}

// Rendering
void WdmStormCloud::PRender(/*VDX9RENDER*/ void* rs)
{
    LRender(rs);
}

void WdmStormCloud::LRender(/*VDX9RENDER*/ void* rs)
{
    // Draw a cloud
    WdmCloud::LRender(rs);
}
