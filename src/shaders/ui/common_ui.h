#pragma once

#ifdef __cplusplus
#include <vector>

#include "../hlslpp.h"
#include "../info.h"
#include "../vertex.h"
namespace storm::shaders::common_ui
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
        .num_uniform_buffers  = 0,
    };
};

#endif

struct VertexInput {
#ifndef __cplusplus
    float3 position: TEXCOORD0;
    float2 tex_coord: TEXCOORD1;
    float4 diffuse: TEXCOORD2;
#else
    float3 position;
    float2 tex_coord;
    float4 diffuse;

    static std::vector<VertexAttribute> attributes()
    {
        return {
            VertexAttribute {.location = 0, .slot = 0, .format = VertexElementFormat::Float3, .offset = offsetof(VertexInput, position)},
            VertexAttribute {.location = 1, .slot = 0, .format = VertexElementFormat::Float2, .offset = offsetof(VertexInput, tex_coord)},
            VertexAttribute {.location = 2, .slot = 0, .format = VertexElementFormat::Float4, .offset = offsetof(VertexInput, diffuse)},
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
    float2 tex_coord: TEXCOORD0;
    float4 diffuse: TEXCOORD1;
    float4 position: SV_Position;
};

struct FragmentInput {
    float2 tex_coord: TEXCOORD0;
    float4 diffuse: TEXCOORD1;
};
#endif

#ifdef __cplusplus
}  // namespace storm::shaders::common_ui
#endif
