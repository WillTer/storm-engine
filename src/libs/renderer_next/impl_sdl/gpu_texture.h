#pragma once

#include <cstdint>
#include <memory>

#include "sdl_fwd.h"

namespace storm
{

struct TextureAsset;
struct ColorTargetInfo;

class GPUTexture final
{
public:
    enum class AddressMode { Repeat, MirroredRepeat, Clamp };

    GPUTexture(std::shared_ptr<SDL_GPUDevice> const& device, TextureAsset const& asset, AddressMode address_mode = AddressMode::Repeat);
    GPUTexture(
        std::shared_ptr<SDL_GPUDevice> const& device,
        uint32_t                              width,
        uint32_t                              height,
        uint32_t                              mip_levels,
        int32_t                               format,
        uint32_t                              usage,
        AddressMode                           address_mode = AddressMode::Repeat);
    ~GPUTexture();

    void set_as_target(ColorTargetInfo& target_info);

    void upload(std::shared_ptr<SDL_GPUCopyPass> const& copy_pass);

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;

    std::pair<uint32_t, uint32_t> get_dimensions() const;

private:
    std::shared_ptr<SDL_GPUTransferBuffer> m_transfer_buffer = nullptr;

    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
    std::shared_ptr<SDL_GPUTexture> m_texture = nullptr;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};

}  // namespace storm
