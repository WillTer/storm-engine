#pragma once

#include <memory>

#include <libs/config/technique.h>

#include "sdl_fwd.h"

namespace storm
{

struct TextureAsset;
struct ColorTargetInfo;

class AssetServer;
class IConfigLoader;

class GPUSampler final
{
public:
    GPUSampler(std::shared_ptr<SDL_GPUDevice> const& device, SamplerInfo const& info);
    ~GPUSampler();

    operator SDL_GPUSampler*() const;

private:
    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
};

}  // namespace storm
