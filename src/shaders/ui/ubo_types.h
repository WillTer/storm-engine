#pragma once

#ifdef __cplusplus
#include "../hlslpp.h"
namespace storm::shaders
{
#define ALIGN16 alignas(16)
#else
#define ALIGN16
#endif

struct UBOVertex {
    float4x4 model;
    float4x4 view_proj;
};

struct UBOFragment {
    float4 color;
};

struct ALIGN16 AnimationUBOVertex {
    int32_t frame;
    int32_t h_frames_count;
    int32_t v_frames_count;
    int16_t flip_h;
    int16_t flip_v;
};

#ifdef __cplusplus
}  // namespace storm::shaders
#endif
