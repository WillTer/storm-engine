#pragma once

#include <memory>

#include "gpu_copy_pass.h"
#include "gpu_render_pass.h"
#include "sdl_fwd.h"

namespace storm
{

class GPUCommandBuffer final
{
public:
    GPUCommandBuffer(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<SDL_Window> const& window);
    ~GPUCommandBuffer();

    auto start_render_pass(std::vector<ColorTargetInfo> const& color_targets, SDL_GPUViewport const& viewport) const
        -> std::unique_ptr<GPURenderPass>;
    auto start_copy_pass() const -> std::unique_ptr<GPUCopyPass>;

    auto get_default_target(bool clear = true) const -> ColorTargetInfo;

    template <typename DataType>
    void push_vertex_uniform_data(uint32_t slot, DataType const& data) const
    {
        push_vertex_uniform_data(slot, &data, sizeof(data));
    }

    void push_vertex_uniform_data(uint32_t slot, void const* data, size_t data_size) const;

    template <typename DataType>
    void push_fragment_uniform_data(uint32_t slot, DataType const& data) const
    {
        push_fragment_uniform_data(slot, &data, sizeof(data));
    }

    void push_fragment_uniform_data(uint32_t slot, void const* data, size_t data_size) const;

private:
    std::shared_ptr<SDL_GPUDevice>        m_device            = nullptr;
    std::shared_ptr<SDL_GPUCommandBuffer> m_cmd_buffer        = nullptr;
    SDL_GPUTexture*                       m_swapchain_texture = nullptr;
};

}  // namespace storm
