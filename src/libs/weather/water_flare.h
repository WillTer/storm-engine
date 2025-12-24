#pragma once

#include "typedef.h"
#include "weather_base.h"

class WaterFlare: public Entity
{
    float*        pfAlpha;
    int32_t       iFlareTex, iFlaresNum;
    WEATHER_BASE* pWeather;

    void GenerateFlares();

public:
    WaterFlare();
    ~WaterFlare() override;

    void SetDevice();
    bool Init() override;
    void Realize(uint32_t Delta_Time) const;
    void Execute(uint32_t Delta_Time);
    bool CreateState(ENTITY_STATE_GEN* state_gen);
    bool LoadState(ENTITY_STATE* state);
    void ProcessMessage(uint32_t iMsg, uint32_t wParam, uint32_t lParam);

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }
};
