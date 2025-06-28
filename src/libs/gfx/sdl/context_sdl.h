#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>

namespace storm::gfx
{

struct ContextSDL {
    std::shared_ptr<SDL_Window>    window = nullptr;
    std::shared_ptr<SDL_GPUDevice> device = nullptr;

    SDL_GPUViewport viewport = {};
};

}  // namespace storm::gfx
