#pragma once

#include "i_render_pass_primitive.h"

namespace storm
{

struct BufferUpdateInfo {
    uint32_t offset;
    uint32_t size;
};

class IBuffer: virtual public IRenderPassPrimitive
{
public:
    ~IBuffer() override = default;

    virtual void update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride) = 0;
};

class IIndexBuffer: virtual public IBuffer
{
public:
    ~IIndexBuffer() override = default;

    virtual void draw_indexed() const = 0;
};

}  // namespace storm
