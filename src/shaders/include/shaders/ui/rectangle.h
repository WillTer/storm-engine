#pragma once

#ifdef __cplusplus
#include <vector>

#include "../cpp/hlslpp.h"
#include "../cpp/vertex.h"

namespace storm::shaders::ui::rectangle
{
#endif

struct VertexInput {
#ifndef __cplusplus
    float2 position : TEXCOORD0;
    float4 diffuse : TEXCOORD1;
#else
    float2 position;
    float4 diffuse;

    static std::vector<VertexAttribute> attributes()
    {
        return {
            VertexAttribute {.location = 0, .slot = 0, .format = VertexElementFormat::Float2, .offset = offsetof(VertexInput, position)},
            VertexAttribute {.location = 1, .slot = 0, .format = VertexElementFormat::Float4, .offset = offsetof(VertexInput, diffuse)},
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
}  // namespace storm::shaders::ui::rectangle
#else
struct VertexOutput {
    float4 diffuse : TEXCOORD1;
    float4 position : SV_Position;
};
#endif
