//============================================================================================
//    Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    CharacterAnimationKipper
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#pragma once

#include <libs/animation/animation.h>
#include <libs/core/entity.h>

class CharacterAnimationKipper: public Entity
{
    // --------------------------------------------------------------------------------------------
    // Construction, destruction
    // --------------------------------------------------------------------------------------------
public:
    CharacterAnimationKipper();
    virtual ~CharacterAnimationKipper();

    // Initialization
    bool Init() override;
    void LockTexture(char const* texture);

    void ProcessStage(Stage, uint32_t) override {}

    // --------------------------------------------------------------------------------------------
    // Encapsulation
    // --------------------------------------------------------------------------------------------
private:
    AnimationService* asr;
    Animation*        aniMan;
    Animation*        aniWoman;

    int32_t lockTextures[16];
    int32_t numLTextures;
};
