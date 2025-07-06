#pragma once

#include <cstdint>

namespace storm
{

class GPUCopyPass;
class GPURenderPass;

class IDrawable
{
public:
    virtual ~IDrawable() = default;

    virtual void update(GPUCopyPass const& copy_pass, uint64_t delta_time) = 0;
    virtual void draw(GPURenderPass const& render_pass) const              = 0;
};

}  // namespace storm
