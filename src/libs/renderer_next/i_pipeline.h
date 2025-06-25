#pragma once

#include <cstdint>

namespace storm
{

struct ShaderInfo {
    uint32_t num_samplers;
    uint32_t num_storage_textures;
    uint32_t num_storage_buffers;
    uint32_t num_uniform_buffers;
};

class IPipeline
{
public:
    virtual ~IPipeline() = default;

    virtual void bind_to_render_pass() const = 0;
};

}  // namespace storm
