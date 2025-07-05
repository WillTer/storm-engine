#pragma once

#ifdef __cplusplus
#include <vector>

#include "../hlslpp.h"
#include "../info.h"
#include "../vertex.h"
namespace storm::shaders::texture_sequence
{

constexpr auto VERTEX_SHADER_INFO = Info {
    .num_samplers         = 0,
    .num_storage_textures = 0,
    .num_storage_buffers  = 0,
    .num_uniform_buffers  = 1,
};

constexpr auto FRAGMENT_SHADER_INFO = Info {
    .num_samplers         = 1,
    .num_storage_textures = 0,
    .num_storage_buffers  = 0,
    .num_uniform_buffers  = 1,
};

#endif

struct VertexInput {
#ifndef __cplusplus
    float4 position: TEXCOORD0;
    uint   index: SV_VertexID;
#else
    float4 position;

    static std::vector<VertexAttribute> attributes()
    {
        return {
            VertexAttribute {.location = 0, .slot = 0, .format = VertexElementFormat::Float4, .offset = offsetof(VertexInput, position)},
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
    float2 tex_coord_next: TEXCOORD1;
    float4 position: SV_Position;
};
#endif

#ifdef __cplusplus
}  // namespace storm::shaders::texture_sequence
#endif
