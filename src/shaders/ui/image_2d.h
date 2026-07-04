#pragma once

#ifdef __cplusplus
#include <vector>

#include "../hlslpp.h"
#include "../info.h"
#include "../vertex.h"
namespace storm::shaders::image_2d
{

struct StageInfo {
    constexpr static auto VERTEX = Info {
        .num_samplers         = 0,
        .num_storage_textures = 0,
        .num_storage_buffers  = 0,
        .num_uniform_buffers  = 1,
    };

    constexpr static auto FRAGMENT = Info {
        .num_samplers         = 1,
        .num_storage_textures = 0,
        .num_storage_buffers  = 0,
        .num_uniform_buffers  = 1,
    };
};
#endif

struct VertexInput {
#ifndef __cplusplus
    float2 position : TEXCOORD0;
    float2 uv : TEXCOORD1;
#else
    float2 position;
    float2 uv;

    static std::vector<VertexAttribute> attributes()
    {
        return {
            VertexAttribute {.location = 0, .slot = 0, .format = VertexElementFormat::Float2, .offset = offsetof(VertexInput, position)},
            VertexAttribute {.location = 1, .slot = 0, .format = VertexElementFormat::Float2, .offset = offsetof(VertexInput, uv)},
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

#ifndef __cplusplus
struct VertexOutput {
    float2 tex_coord : TEXCOORD0;
    float4 position : SV_Position;
};
#endif

#ifdef __cplusplus
}  // namespace storm::shaders::image_2d
#endif
