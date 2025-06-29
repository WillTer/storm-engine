#pragma once

#include <memory>
#include <span>

#include "sdl_fwd.h"

namespace storm
{

struct TxFileHeader;

class GPUTexture final
{
public:
    GPUTexture(std::shared_ptr<SDL_GPUDevice> const& device, TxFileHeader const& file_header);
    GPUTexture(
        std::shared_ptr<SDL_GPUDevice> const& device, uint32_t width, uint32_t height, uint32_t mip_levels, int32_t format, uint32_t usage);
    ~GPUTexture();

    template <typename T>
    void
    upload(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, std::span<T> const& data) const
    {
        upload(device, copy_pass, data.data(), static_cast<uint32_t>(data.size() * sizeof(T)));
    }

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;

    std::pair<uint32_t, uint32_t> get_dimensions() const;

private:
    void upload(
        std::shared_ptr<SDL_GPUDevice> const&   device,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        void const*                             data,
        uint32_t                                data_size) const;

    std::shared_ptr<SDL_GPUSampler> m_sampler = nullptr;
    std::shared_ptr<SDL_GPUTexture> m_texture = nullptr;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};

}  // namespace storm
