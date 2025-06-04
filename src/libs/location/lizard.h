//============================================================================================
//    Spirenkov Maxim
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    Lizard
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#pragma once

#include "loc_life.h"

class Lizard: public LocLife
{
public:
    Lizard();
    ~Lizard() override;

private:
    char const* GetModelName() override;
    char const* GetAniName() override;
    bool        PostInit(Animation* ani) override;

    void IdleProcess(Animation* ani, float dltTime) override;
    void MoveProcess(Animation* ani, float dltTime) override;
    void IsStartMove(Animation* ani) override;
    void IsStopMove(Animation* ani) override;

private:
    float lastMove;
};
