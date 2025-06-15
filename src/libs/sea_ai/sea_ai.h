#pragma once

#include "ai_balls.h"
#include "ai_fort.h"
#include "ai_helper.h"

class SeaAI: public Entity
{
private:
    bool bFirstInit;

    void AddShip(entid_t _eidShip, ATTRIBUTES* pCharacter, ATTRIBUTES* pAShip);
    void SetCompanionEnemy(ATTRIBUTES* pACharacter);

public:
    SeaAI();
    ~SeaAI() override;

    bool Init() override;
    void SetDevice();
    void Save(char const* pName);
    void Load(char const* pName);

    void Realize(uint32_t Delta_Time);
    void Execute(uint32_t Delta_Time);

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

    bool CreateState(ENTITY_STATE_GEN* state_gen);
    bool LoadState(ENTITY_STATE* state);

    void     ProcessMessage(uint32_t iMsg, uint32_t wParam, uint32_t lParam);
    uint64_t ProcessMessage(MESSAGE& message) override;

    uint32_t AttributeChanged(ATTRIBUTES* pAttribute) override;
};
