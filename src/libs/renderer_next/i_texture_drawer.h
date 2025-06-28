#pragma once

#include <cstdint>

namespace storm
{

class ITexture;

class ITextureDrawer
{
public:
    virtual ~ITextureDrawer() = default;

    virtual void update(uint64_t delta_time)     = 0;
    virtual void present(ITexture& source) const = 0;
};

}  // namespace storm
