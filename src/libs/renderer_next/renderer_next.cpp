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

void RendererNext::bind_window(InternalWindowType const& window)
{
    if (!std::holds_alternative<SDL_Window*>(window)) { throw std::runtime_error("Only SDL window is supported for SDL_GPU API"); }

    auto* sdl_window = std::get<SDL_Window*>(window);
    if (!SDL_ClaimWindowForGPUDevice(m_device.get(), sdl_window)) {
        throw std::runtime_error(std::format("Can't claim window for device: {}", SDL_GetError()));
    }
}

void RendererNext::unbind_window(InternalWindowType const& window)
{
    if (!std::holds_alternative<SDL_Window*>(window)) { throw std::runtime_error("Only SDL window is supported for SDL_GPU API"); }

    auto* sdl_window = std::get<SDL_Window*>(window);
    SDL_ReleaseWindowFromGPUDevice(m_device.get(), sdl_window);
}
