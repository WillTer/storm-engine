#pragma once

namespace storm
{

class ITexture
{
public:
    virtual ~ITexture() = default;

    virtual void bind_to_render_pass() const = 0;
};

}  // namespace storm
