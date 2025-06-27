#pragma once

#include <cstdint>
#include <utility>

#include "i_render_pass_primitive.h"

namespace storm
{

class ITexture: virtual public IRenderPassPrimitive
{
public:
    ~ITexture() override = default;

    virtual std::pair<uint32_t, uint32_t> get_dimensions() const = 0;
};

class ITextureTarget: virtual public ITexture
{
public:
    virtual ~ITextureTarget() = default;

    virtual void start_render_pass(bool clear = true) = 0;
    virtual void end_render_pass()                    = 0;
};

}  // namespace storm
