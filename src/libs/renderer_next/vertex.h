#pragma once

#include "types.h"

namespace storm
{

enum class VertexElementFormat {
    Unknown,

    Int,
    Int2,
    Int3,
    Int4,

    UInt,
    UInt2,
    UInt3,
    UInt4,

    Float,
    Float2,
    Float3,
    Float4,

    Byte2,
    Byte4,

    UByte2,
    UByte4,

    Byte2Norm,
    Byte4Norm,

    UByte2Norm,
    UByte4Norm,

    Short2,
    Short4,

    UShort2,
    UShort4,

    Short2Norm,
    Short4Norm,

    UShort2Norm,
    UShort4Norm,

    Half2,
    Half4,
};

enum class VertexInputRate { Vertex, Instance };

struct VertexAttribute {
    uint32_t            location;
    uint32_t            slot;
    VertexElementFormat format;
    uint32_t            offset;
};

struct VertexDescription {
    uint32_t        slot;
    uint32_t        stride;
    VertexInputRate input_rate;
    uint32_t        instance_step_rate;
};

struct VertexBase {
    FPoint3D position;
    FPoint   uv;

    static std::vector<VertexAttribute> attributes()
    {
        return {
            VertexAttribute {.location = 0, .slot = 0, .format = VertexElementFormat::Float3, .offset = offsetof(VertexBase, position)},
            VertexAttribute {.location = 1, .slot = 0, .format = VertexElementFormat::Float2, .offset = offsetof(VertexBase, uv)},
        };
    }

    static std::vector<VertexDescription> descriptions()
    {
        return {
            VertexDescription {.slot = 0, .stride = sizeof(VertexBase), .input_rate = VertexInputRate::Vertex, .instance_step_rate = 0},
        };
    }
};

struct VertexWithDiffuse {
    VertexBase base;
    FColor     diffuse;

    static std::vector<VertexAttribute> attributes()
    {
        auto base_attributes = VertexBase::attributes();
        base_attributes.emplace_back(
            VertexAttribute {
                .location = 2, .slot = 0, .format = VertexElementFormat::Float4, .offset = offsetof(VertexWithDiffuse, diffuse)});

        return base_attributes;
    }

    static std::vector<VertexDescription> descriptions()
    {
        return {
            VertexDescription {
                .slot = 0, .stride = sizeof(VertexWithDiffuse), .input_rate = VertexInputRate::Vertex, .instance_step_rate = 0},
        };
    }
};

}  // namespace storm
