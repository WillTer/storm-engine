#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>

namespace storm::gfx
{

struct CommandBufferSDL {
    std::shared_ptr<SDL_GPUCommandBuffer> cmd = nullptr;

    SDL_GPUTexture* swapchain_texture = nullptr;
};

}  // namespace storm::gfx
