#pragma once

#include <cstdint>

namespace storm
{

class GPUCopyPass;
class GPUCommandBuffer;

class IPreDrawable
{
public:
    virtual ~IPreDrawable() = default;

    virtual void update(GPUCopyPass const& copy_pass, uint64_t delta_time)               = 0;
    virtual void pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t delta_time) const = 0;
};

}  // namespace storm
