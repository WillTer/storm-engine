#pragma once

#include <filesystem>
#include <memory>

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/texture_asset.h>
#include <libs/renderer_next/i_texture.h>

namespace storm
{

class RendererSDL;
class TextureSDL final: virtual public ITexture
{
public:
    TextureSDL(RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, TextureAsset const& asset);
    ~TextureSDL() override;

    void bind_to_render_pass() const override;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
    std::shared_ptr<SDL_GPUTexture> m_texture = nullptr;
};

}  // namespace storm
