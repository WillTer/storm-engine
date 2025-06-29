#include "gpu_command_buffer.h"

#include <SDL3/SDL_gpu.h>
#include <spdlog/spdlog.h>

#include "libs/renderer_next/impl_sdl/gpu_render_pass.h"

using namespace storm;

GPUCommandBuffer::GPUCommandBuffer(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<SDL_Window> const& window)
    : m_device(device)
{
    m_cmd_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(device.get()), &SDL_SubmitGPUCommandBuffer);
    if (!m_cmd_buffer) { spdlog::error("Acquire GPU command buffer failed: {}", SDL_GetError()); }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_cmd_buffer.get(), window.get(), &m_swapchain_texture, nullptr, nullptr)
        || m_swapchain_texture == nullptr) {
        spdlog::error("Acquire GPU swapchain texture failed: {}", SDL_GetError());
    }
}

GPUCommandBuffer::~GPUCommandBuffer() = default;

auto GPUCommandBuffer::start_render_pass(std::vector<ColorTargetInfo> const& color_targets, SDL_GPUViewport const& viewport) const
    -> std::unique_ptr<GPURenderPass>
{
    return std::make_unique<GPURenderPass>(m_cmd_buffer, color_targets, viewport);
}

auto GPUCommandBuffer::start_copy_pass() const -> std::unique_ptr<GPUCopyPass>
{
    return std::make_unique<GPUCopyPass>(m_device, m_cmd_buffer);
}

auto GPUCommandBuffer::get_default_target(bool clear /*= true*/) const -> ColorTargetInfo
{
    return ColorTargetInfo {
        .target_texture = m_swapchain_texture,
        .clear_color    = {0.0F, 0.0F, 0.0F, 1.0F},  // black
        .load_op        = clear ? LoadOp::Clear : LoadOp::Load,
        .store_op       = StoreOp::Store,
    };
}

void GPUCommandBuffer::push_vertex_uniform_data(uint32_t const slot, void const* data, size_t const data_size) const
{
    SDL_PushGPUVertexUniformData(m_cmd_buffer.get(), slot, data, static_cast<uint32_t>(data_size));
}

void GPUCommandBuffer::push_fragment_uniform_data(uint32_t const slot, void const* data, size_t const data_size) const
{
    SDL_PushGPUFragmentUniformData(m_cmd_buffer.get(), slot, data, static_cast<uint32_t>(data_size));
}
