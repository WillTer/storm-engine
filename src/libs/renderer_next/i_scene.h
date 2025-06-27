#pragma once

#include <cstdint>
#include <memory>

namespace storm
{

class ITexture;
class ITextureTarget;
class IPostProcessor;

class IScene
{
public:
    virtual ~IScene() = default;

    virtual void update(uint64_t delta_time) = 0;
    virtual void render() const              = 0;

    virtual void set_post_processor(std::shared_ptr<IPostProcessor> const& post_processor) = 0;

    virtual std::shared_ptr<IPostProcessor> get_post_processor() const = 0;
};

}  // namespace storm
