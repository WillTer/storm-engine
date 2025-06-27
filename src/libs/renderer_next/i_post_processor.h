#pragma once

#include <cstdint>
#include <memory>

namespace storm
{

class ITexture;
class ITextureTarget;

class IPostProcessor
{
public:
    virtual ~IPostProcessor() = default;

    virtual void update(uint64_t delta_time)          = 0;
    virtual void render(ITexture& scene_target) const = 0;

    virtual void set_next(std::shared_ptr<IPostProcessor> const& next) = 0;

    virtual std::shared_ptr<IPostProcessor> get_next() const = 0;
};

}  // namespace storm
