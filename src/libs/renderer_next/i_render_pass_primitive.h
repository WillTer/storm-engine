#pragma once

namespace storm
{

class IRenderPassPrimitive
{
public:
    virtual ~IRenderPassPrimitive() = default;

    virtual void bind_to_render_pass() const = 0;
};

}  // namespace storm
