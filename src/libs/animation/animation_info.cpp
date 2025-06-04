// ============================================================================================
// Spirenkov Maxim aka Sp-Max Shaman, 2001
// --------------------------------------------------------------------------------------------
// Storm engine v2.00
// --------------------------------------------------------------------------------------------
// AnimationInfo
// --------------------------------------------------------------------------------------------
// This is where animation information is stored, loaded 1 time
// ============================================================================================

#include "animation_info.h"

#include <libs/util/platform/platform.hpp>
#include <libs/util/string_compare.hpp>

// ============================================================================================
// Construction, destruction
// ============================================================================================

AnimationInfo::AnimationInfo(char const* animationName) : numFrames(0)
{
    Assert(strlen(animationName) < 64);
    strcpy_s(name, animationName);
    bone       = nullptr;
    numBones   = 0;
    refCounter = 0;
    downtime   = 0;
    fps        = 15.0f;
}

AnimationInfo::~AnimationInfo()
{
    delete[] bone;
}

// Create bones
void AnimationInfo::CreateBones(int32_t numbones)
{
    Assert(bone == nullptr || numBones == 0);
    Assert(numbones > 0 && numbones <= 256);
    numBones = numbones;
    bone     = new Bone[numBones];
}

// Create action
ActionInfo* AnimationInfo::AddAction(char const* anctionName, int32_t startframe, int32_t endframe)
{
    Assert(anctionName);
    // Looking for repetition
    for (auto const& action: actions)
        if (action == anctionName) return nullptr;

    // It's okay - new action
    return &actions.emplace_back(anctionName, startframe, endframe);
}

// --------------------------------------------------------------------------------------------
// Working with animation
// --------------------------------------------------------------------------------------------

// Compare with current name
bool AnimationInfo::operator==(char const* animationName)
{
    return storm::iEquals(animationName, name);
}

// Find action by name
ActionInfo* AnimationInfo::FindAction(char const* actionName)
{
    for (auto& action: actions)
        if (action == actionName) return &action;
    return nullptr;
}
