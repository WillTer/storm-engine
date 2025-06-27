#pragma once

#include <filesystem>
#include <memory>

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/texture_asset.h>
#include <libs/renderer_next/i_texture.h>

namespace storm
{

class RendererSDL;
class TextureSDL final: virtual public ITextureTarget
{
public:
    TextureSDL(RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, TextureAsset const& asset);
    TextureSDL(
        RendererSDL&             renderer,
        uint32_t                 width,
        uint32_t                 height,
        uint32_t                 mip_levels,
        SDL_GPUTextureFormat     format,
        SDL_GPUTextureUsageFlags usage);
    ~TextureSDL() override;

    // IRenderPassPrimitive
    void bind_to_render_pass() const override;

    // ITexture
    std::pair<uint32_t, uint32_t> get_dimensions() const override;

    // ITextureTarget
    void start_render_pass(bool clear = true) override;
    void end_render_pass() override;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
    std::shared_ptr<SDL_GPUTexture> m_texture = nullptr;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};

}  // namespace storm
