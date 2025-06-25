#pragma once

#include "i_render_pass_primitive.h"

namespace storm
{

class IBuffer: virtual public IRenderPassPrimitive
{
public:
    ~IBuffer() override = default;
};

class IIndexBuffer: virtual public IBuffer
{
public:
    ~IIndexBuffer() override = default;

    virtual void draw_indexed() const = 0;
};

}  // namespace storm
