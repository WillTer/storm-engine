#pragma once

#include <cstdint>
#include <memory>
#include <optional>

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
    GPUSampler(
        std::shared_ptr<SDL_GPUDevice> const& device,
        std::shared_ptr<AssetServer> const&   asset_server,
        std::shared_ptr<IConfigLoader> const& config_loader,
        std::string const&                    technique);
    ~GPUSampler();

    operator SDL_GPUSampler*() const;

private:
    void initialize(std::shared_ptr<SDL_GPUDevice> const& device, SamplerInfo const& info);

    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
};

}  // namespace storm
