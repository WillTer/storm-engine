//============================================================================================
//    Spirenkov Maxim, 2003
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    Fader
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#pragma once

#include <libs/core/entity.h>

namespace storm
{
class FaderPostProcess;
}

class Fader final: public Entity
{
    // --------------------------------------------------------------------------------------------
    // Construction, destruction
    // --------------------------------------------------------------------------------------------
public:
    Fader();
    ~Fader() override;

    // Initialization
    bool Init() override;
    // Messages
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage const stage, uint32_t const delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        default: break;
        }
    }

    // Work
    void Execute(uint32_t delta_time);
    void Realize(uint32_t delta_time);

    // --------------------------------------------------------------------------------------------
    // Encapsulation
    // --------------------------------------------------------------------------------------------
private:
    bool fadeIn;
    bool isStart;
    bool isAutodelete;

    bool    eventStart;
    bool    eventEnd;
    int32_t deleteMe;

    std::shared_ptr<storm::FaderPostProcess> m_fader_render;

public:
    static int32_t numberOfTips;
    static int32_t currentTips;
};
