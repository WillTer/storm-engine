#include "context_sdl.h"

#include <cassert>
#include <string>
#include <unordered_map>

#include <libs/asset_server/asset_server.h>
#include <libs/config/main_config.h>
#include <libs/gfx/context.h>
#include <spdlog/spdlog.h>

namespace
{
#ifdef _DEBUG
constexpr bool IS_DEBUG_MODE = true;
#else
constexpr bool IS_DEBUG_MODE = false;
#endif

#ifdef _WIN32
constexpr std::string_view DEFAULT_BACKEND    = "direct3d12";
auto const                 BACKEND_SHADER_EXT = std::unordered_map<std::string, std::string> {
    {"direct3d12", "dxil"},
    {"vulkan", "spv"},
};
#else
constexpr std::string_view DEFAULT_BACKEND    = "vulkan";
auto const                 BACKEND_SHADER_EXT = std::unordered_map<std::string, std::string> {
    {"vulkan", "spv"},
};
#endif

}  // namespace

namespace storm::gfx
{

template <>
void init_device(ContextSDL& ctx, std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader)
{
    assert(asset_server);
    assert(config_loader);

    auto const device_info = main_config::device_info(*config_loader);

    auto backend = device_info.backend;
    if (!BACKEND_SHADER_EXT.contains(backend)) {
        spdlog::info("Unknown backend value in [device] settings: \"{}\", fallback to \"{}\"", backend, DEFAULT_BACKEND);
        backend = DEFAULT_BACKEND;
    }

    ctx.device = std::shared_ptr<SDL_GPUDevice>(
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV, IS_DEBUG_MODE, backend.c_str()), &SDL_DestroyGPUDevice);

    if (!ctx.device) { throw std::runtime_error(std::format("Failed to create GPU device: {}", SDL_GetError())); }

    asset_server->set_asset_ext<ShaderAsset>(BACKEND_SHADER_EXT.at(backend));

    ctx.viewport           = {};
    ctx.viewport.max_depth = 1.0F;
}

template <>
void claim_window(ContextSDL& ctx, std::shared_ptr<SDL_Window> const& raw_window)
{
    assert(raw_window);

    ctx.window = raw_window;

    if (!SDL_ClaimWindowForGPUDevice(ctx.device.get(), ctx.window.get())) {
        throw std::runtime_error(std::format("Can't claim window for device: {}", SDL_GetError()));
    }

    int width  = 0;
    int height = 0;
    SDL_GetWindowSize(ctx.window.get(), &width, &height);

    ctx.viewport.w = static_cast<float>(width);
    ctx.viewport.h = static_cast<float>(height);
}

template <>
void unclaim_window(ContextSDL& ctx, std::shared_ptr<SDL_Window> const& raw_window)
{
    if (raw_window != ctx.window) {
        spdlog::warn("Trying to unclain wrong window from device");
        return;
    }

    SDL_ReleaseWindowFromGPUDevice(ctx.device.get(), ctx.window.get());
    ctx.window.reset();
}

}  // namespace storm::gfx
