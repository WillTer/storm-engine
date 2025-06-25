#pragma once

namespace storm
{

class IBuffer
{
public:
    virtual ~IBuffer() = default;

    virtual void bind_to_render_pass() const = 0;
};

}  // namespace storm
