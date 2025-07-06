#pragma once

#ifdef __cplusplus
#include "../hlslpp.h"
namespace storm::shaders
{
#define ALIGN16 alignas(16)
#else
#define ALIGN16
#endif

struct ALIGN16 UBOVertex {
    float4x4 model;
    float4x4 view_proj;
};

struct ALIGN16 UBOFragment {
    float4 color;
};

struct ALIGN16 AnimationUBOVertex {
    int frame;
    int h_frames_count;
    int v_frames_count;
    int flip_h;
    int flip_v;
};

#ifdef __cplusplus
}  // namespace storm::shaders
#endif
