#pragma once

#include "i_render_pass_primitive.h"

namespace storm
{

class ITexture: virtual public IRenderPassPrimitive
{
public:
    ~ITexture() override = default;

    virtual std::pair<uint32_t, uint32_t> get_dimensions() const = 0;
};

}  // namespace storm
