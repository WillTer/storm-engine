#pragma once

#include "i_render_pass_primitive.h"

namespace storm
{

class IPipeline: virtual public IRenderPassPrimitive
{
public:
    ~IPipeline() override = default;
};

}  // namespace storm
