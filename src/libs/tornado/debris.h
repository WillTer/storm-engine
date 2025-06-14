//============================================================================================
//    Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    Debris
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#pragma once

#include <libs/math/matrix.h>
#include <libs/model/model.h>
#include <libs/sound_service/v_sound_service.h>

#include "pillar.h"

class Debris
{
    struct ModelInfo {
        float   a, r;
        float   y, ay;
        float   alpha;
        float   scale;
        float   maxSpeed;
        CVECTOR ang;
        MODEL*  mdl;
    };

    struct Model {
        MODEL* mdl;
        float  prt;
        float  maxSpeed;
    };

    // --------------------------------------------------------------------------------------------
    // Construction, destruction
    // --------------------------------------------------------------------------------------------
public:
    Debris(Pillar& _pillar);
    virtual ~Debris();

    void Init(std::shared_ptr<entt::registry> const& registry);

    void Update(float dltTime);
    void Draw(VDX9RENDER* rs);

    void SetGlobalAlpha(float a);

    // --------------------------------------------------------------------------------------------
    // Encapsulation
    // --------------------------------------------------------------------------------------------
private:
    void   AddModel(char const* modelNamem, float prt, float spd);
    void   NormalazedModels();
    MODEL* SelectModel(float& maxSpd);
    bool   IsShip();

private:
    std::shared_ptr<entt::registry> m_registry;

    float   lastPlayTime;
    Pillar& pillar;
    float   galpha;
    Model   mdl[16];    // Uploaded models
    int32_t numModels;  // Number of models

    ModelInfo fly[64];     // Flying models
    int32_t   flyCounter;  // Number of flying models
};
