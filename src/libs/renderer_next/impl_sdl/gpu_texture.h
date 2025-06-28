#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/texture_asset.h>

namespace storm
{

class RendererService;
class GPUTexture final
{
public:
    GPUTexture(RendererService& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, TextureAsset const& asset);
    GPUTexture(
        RendererService&         renderer,
        uint32_t                 width,
        uint32_t                 height,
        uint32_t                 mip_levels,
        SDL_GPUTextureFormat     format,
        SDL_GPUTextureUsageFlags usage);
    ~GPUTexture();

    // IRenderPassPrimitive
    void bind_to_render_pass() const;

    // ITexture
    std::pair<uint32_t, uint32_t> get_dimensions() const;

    // ITextureTarget
    void start_render_pass(bool clear = true);
    void end_render_pass();

private:
    RendererService& m_renderer;

    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
    std::shared_ptr<SDL_GPUTexture> m_texture = nullptr;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};

}  // namespace storm
