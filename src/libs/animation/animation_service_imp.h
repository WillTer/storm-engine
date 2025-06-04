// ============================================================================================
// Spirenkov Maxim aka Sp-Max Shaman, 2001
// --------------------------------------------------------------------------------------------
// Storm engine v2.00
// --------------------------------------------------------------------------------------------
// AnimationServiceImp
// --------------------------------------------------------------------------------------------
// Animation service for creating AnimationManager objects
// ============================================================================================

#pragma once

#include <libs/core/vma.hpp>

#include "animation.h"
#include "animation_info.h"

//============================================================================================

class INIFILE;
class AnimationImp;

class AnimationServiceImp final: public AnimationService
{
    // --------------------------------------------------------------------------------------------
    // Construction, destruction
    // --------------------------------------------------------------------------------------------
public:
    AnimationServiceImp();
    ~AnimationServiceImp() override;

    // Phase for running the animation
    uint32_t RunSection() override;
    // Execution functions
    void RunStart() override;
    void RunEnd() override;
    // Create animation for the model, delete using "delete"
    Animation* CreateAnimation(char const* animationName) override;

    // --------------------------------------------------------------------------------------------
    // Functions for Animation
    // --------------------------------------------------------------------------------------------
    // Remove animation (called from destructor)
    void DeleteAnimation(AnimationImp* ani);
    // Event
    void Event(char const* eventName);

    // --------------------------------------------------------------------------------------------
    // Encapsulation
    // --------------------------------------------------------------------------------------------
private:
    // load animation
    int32_t LoadAnimation(char const* animationName);
    // Load user data from the current section
    void LoadUserData(INIFILE* ani, char const* sectionName, std::unordered_map<std::string, std::string>& data, char const* animationName);
    // load AN
    bool LoadAN(char const* fname, AnimationInfo* info);

    std::vector<AnimationInfo*> ainfo;
    std::vector<AnimationImp*>  animations;

    static char key[1024];
};

//============================================================================================
