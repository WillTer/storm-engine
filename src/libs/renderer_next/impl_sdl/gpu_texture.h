#pragma once

#include <cstdint>
#include <memory>

#include "sdl_fwd.h"

namespace storm
{

struct TextureAsset;
struct ColorTargetInfo;

class GPUSampler;

class GPUTexture final
{
public:
    enum class AddressMode { Repeat, MirroredRepeat, Clamp };

    GPUTexture(std::shared_ptr<SDL_GPUDevice> const& device, TextureAsset const& asset);
    GPUTexture(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<GPUSampler> const& sampler, TextureAsset const& asset);

    GPUTexture(
        std::shared_ptr<SDL_GPUDevice> const& device,
        std::shared_ptr<GPUSampler> const&    sampler,
        uint32_t                              width,
        uint32_t                              height,
        uint32_t                              mip_levels,
        int32_t                               format,
        uint32_t                              usage);

    ~GPUTexture();

    void set_as_target(ColorTargetInfo& target_info);

    void upload(std::shared_ptr<SDL_GPUCopyPass> const& copy_pass);

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;

    void set_sampler(std::shared_ptr<GPUSampler> const& sampler);

    std::pair<uint32_t, uint32_t> get_dimensions() const;

private:
    std::shared_ptr<SDL_GPUTransferBuffer> m_transfer_buffer = nullptr;

    std::shared_ptr<SDL_GPUTexture> m_texture = nullptr;
    std::shared_ptr<GPUSampler>     m_sampler = nullptr;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};

}  // namespace storm
