#pragma once

#include <cstdint>

namespace storm::shaders
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

}  // namespace storm::shaders
