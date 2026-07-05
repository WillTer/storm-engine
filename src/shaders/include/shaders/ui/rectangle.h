#pragma once

#ifdef __cplusplus
#include <vector>

#include "../cpp/hlslpp.h"
#include "../cpp/vertex.h"

namespace storm::shaders::rectangle
{
#endif

struct VertexInput {
#ifndef __cplusplus
    float2 position : TEXCOORD0;
    float4 color : TEXCOORD1;
#else
    float2 position;
    float4 color;

    static std::vector<VertexAttribute> attributes()
    {
        return {
            VertexAttribute {.location = 0, .slot = 0, .format = VertexElementFormat::Float2, .offset = offsetof(VertexInput, position)},
            VertexAttribute {.location = 1, .slot = 0, .format = VertexElementFormat::Float4, .offset = offsetof(VertexInput, color)},
        };
    }

    static std::vector<VertexDescription> descriptions()
    {
        return {
            VertexDescription {.slot = 0, .stride = sizeof(VertexInput), .input_rate = VertexInputRate::Vertex, .instance_step_rate = 0},
        };
    }
#endif
};

#ifdef __cplusplus
}  // namespace storm::shaders::rectangle
#else
struct VertexOutput {
    float4 color : TEXCOORD0;
    float4 position : SV_Position;
};
#endif
