#include "renderer_next.h"

#include <format>
#include <memory>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <libs/window/sdl_window.hpp>

using namespace storm;

namespace
{
#ifdef _DEBUG
constexpr bool is_debug_mode = true;
#else
constexpr bool is_debug_mode = false;
#endif
}  // namespace

RendererNext::RendererNext()
{
    m_device = std::shared_ptr<SDL_GPUDevice>(
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, is_debug_mode, nullptr),
        &SDL_DestroyGPUDevice);

    if (!m_device) { throw std::runtime_error(std::format("Failed to create GPU device: {}", SDL_GetError())); }
}

RendererNext::~RendererNext() = default;

void RendererNext::bind_window(std::shared_ptr<OSWindow> const& window)
{
    auto* sdl_window = dynamic_cast<SDLWindow*>(window.get());
    if (sdl_window == nullptr) { throw std::runtime_error("Only SDL window is supported for SDL GPU"); }

    if (!SDL_ClaimWindowForGPUDevice(m_device.get(), sdl_window->SDLHandle())) {
        throw std::runtime_error(std::format("Can't claim window for device: {}", SDL_GetError()));
    }
}
