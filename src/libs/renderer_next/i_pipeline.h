#pragma once

#include <string>

#include "types.h"

namespace storm
{

enum class ShaderStage { Vertex, Fragment };

struct Position {
    FPoint3D position;
};

struct PositionTexture {
    FPoint3D position;
    FPoint   uv;
};

struct PositionTextureColor {
    FPoint3D position;
    FPoint   uv;
    Color    diffuse;
};

struct ShaderInfo {
    std::string file_name;
    ShaderStage stage;
    uint32_t    num_samplers;
    uint32_t    num_storage_textures;
    uint32_t    num_storage_buffers;
    uint32_t    num_uniform_buffers;
};

class IPipeline
{
public:
    virtual ~IPipeline() = default;

    virtual void present() const = 0;
};

}  // namespace storm
