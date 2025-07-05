#pragma once

#include <cstdint>

namespace storm::shaders
{

struct Info {
    uint32_t num_samplers;
    uint32_t num_storage_textures;
    uint32_t num_storage_buffers;
    uint32_t num_uniform_buffers;
};

}  // namespace storm::shaders
