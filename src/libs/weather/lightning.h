#pragma once

#include <vector>

#include "typedef.h"
#include "weather_base.h"

#define MAX_LIGHTNING_TEXTURES 2

class Lightning: public Entity
{
    struct flash_t {
        std::string sTechnique;
        float       fSize;
        float       fTime;
    };

    struct lightning_t {
        // script parameters
        uint32_t dwSubTexture;
        uint32_t dwFlickerTime;
        float    fTime;
        float    fSize, fScaleX, fScaleY;
        CVECTOR  vPos;

        // inner parameters
        float fAlpha;
        float fPower;

        std::string sTechnique;

        flash_t Flash;
    };

    std::vector<lightning_t> aLightnings;

    uint32_t dwSubTexX, dwSubTexY;
    int32_t  iLightningTexture, iFlashTexture;

    float   fKDist;
    int32_t iFlickerTime;

    WEATHER_BASE* pWeather;
    COLLIDE*      pCollide;

    void Release() const;
    void CalcFlashPower(lightning_t* pL) const;

public:
    Lightning();
    ~Lightning() override;

    void     SetDevice();
    bool     Init() override;
    void     Realize(uint32_t Delta_Time);
    void     Execute(uint32_t Delta_Time);
    bool     CreateState(ENTITY_STATE_GEN* state_gen);
    bool     LoadState(ENTITY_STATE* state);
    uint64_t ProcessMessage(MESSAGE& message) override;
    uint32_t AttributeChanged(ATTRIBUTES* pAttributeChanged) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }
};
