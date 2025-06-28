#include "command_buffer_sdl.h"

#include <libs/gfx/command_buffer.h>
#include <spdlog/spdlog.h>

#include "context_sdl.h"

namespace storm::gfx
{

template <>
std::unique_ptr<CommandBufferSDL> acquire_cmd_buffer(ContextSDL& ctx)
{
    auto cmd_buffer = std::make_unique<CommandBufferSDL>();

    cmd_buffer->cmd = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(ctx.device.get()), &SDL_SubmitGPUCommandBuffer);
    if (!cmd_buffer->cmd) {
        spdlog::error("Acquire GPU command buffer failed: {}", SDL_GetError());
        return nullptr;
    }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buffer->cmd.get(), ctx.window.get(), &cmd_buffer->swapchain_texture, nullptr, nullptr)
        || cmd_buffer->swapchain_texture == nullptr) {
        spdlog::error("Acquire GPU swapchain texture failed: {}", SDL_GetError());
        return nullptr;
    }

    return cmd_buffer;
}

}  // namespace storm::gfx
