#pragma once

#include "i_render_pass_primitive.h"

namespace storm
{

class ITexture: virtual public IRenderPassPrimitive
{
public:
    ~ITexture() override = default;
};

}  // namespace storm
