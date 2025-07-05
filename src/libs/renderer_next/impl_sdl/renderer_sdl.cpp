#include "renderer_sdl.h"

#include <format>
#include <memory>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/asset_server.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/config/main_config.h>
#include <libs/core/core.h>
#include <libs/renderer_next/cache.h>
#include <libs/window/sdl_window.hpp>
#include <spdlog/spdlog.h>

#include "create_pipeline.h"
#include "gpu_command_buffer.h"
#include "gpu_index_buffer.h"
#include "gpu_texture.h"
#include "gpu_vertex_buffer.h"

using namespace storm;

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

struct RendererService::Impl {
    Impl(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader)
        : m_asset_server(asset_server)
    {
        assert(m_asset_server);
        assert(config_loader);

        auto const device_info = main_config::device_info(*config_loader);

        auto backend = device_info.backend;
        if (!BACKEND_SHADER_EXT.contains(backend)) {
            spdlog::info("Unknown backend value in [device] settings: \"{}\", fallback to \"{}\"", backend, DEFAULT_BACKEND);
            backend = DEFAULT_BACKEND;
        }

        m_device = std::shared_ptr<SDL_GPUDevice>(
            SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV, IS_DEBUG_MODE, backend.c_str()),
            &SDL_DestroyGPUDevice);

        if (!m_device) { throw std::runtime_error(std::format("Failed to create GPU device: {}", SDL_GetError())); }

        m_asset_server->set_asset_ext<ShaderAsset>(BACKEND_SHADER_EXT.at(backend));

        m_viewport.min_depth = 0.0F;
        m_viewport.max_depth = 1.0F;
    }

    ~Impl() = default;

    void bind_window(std::shared_ptr<SDL_Window> const& raw_window)
    {
        m_window = raw_window;

        if (!SDL_ClaimWindowForGPUDevice(m_device.get(), m_window.get())) {
            throw std::runtime_error(std::format("Can't claim window for device: {}", SDL_GetError()));
        }

        int width  = 0;
        int height = 0;
        SDL_GetWindowSize(m_window.get(), &width, &height);

        m_viewport.w = static_cast<float>(width);
        m_viewport.h = static_cast<float>(height);
    }

    void unbind_window(std::shared_ptr<SDL_Window> const& raw_window)
    {
        if (raw_window != m_window) {
            spdlog::warn("Trying to unbind wrong window from renderer");
            return;
        }

        SDL_ReleaseWindowFromGPUDevice(m_device.get(), m_window.get());
        m_window = nullptr;
    }

    [[nodiscard]] auto create_texture(entt::hashed_string const& name, TxFileHeader const& file_header) -> std::shared_ptr<GPUTexture>
    {
        if (!m_cache.contains<GPUTexture>(name)) { m_cache.add(name, std::make_shared<GPUTexture>(m_device, file_header)); }

        return m_cache.get<GPUTexture>(name);
    }

    [[nodiscard]] auto create_texture_target(uint32_t const width, uint32_t const height) -> std::unique_ptr<GPUTexture>
    {
        int window_width  = 0;
        int window_height = 0;
        SDL_GetWindowSize(m_window.get(), &window_width, &window_height);
        return std::make_unique<GPUTexture>(
            m_device,
            width > 0 ? width : window_width,
            height > 0 ? height : window_height,
            1,
            SDL_GetGPUSwapchainTextureFormat(m_device.get(), m_window.get()),
            SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
    }

    [[nodiscard]] auto create_index_buffer(size_t const index_count) -> std::unique_ptr<GPUIndexBuffer>
    {
        return std::make_unique<GPUIndexBuffer>(m_device, static_cast<uint32_t>(index_count));
    }

    [[nodiscard]] auto create_vertex_buffer(size_t const vertex_count, size_t const vertex_type_size) -> std::unique_ptr<GPUVertexBuffer>
    {
        return std::make_unique<GPUVertexBuffer>(m_device, static_cast<uint32_t>(vertex_count), static_cast<uint32_t>(vertex_type_size));
    }

    auto acquire_command_buffer() const -> std::unique_ptr<GPUCommandBuffer>
    {
        return std::make_unique<GPUCommandBuffer>(m_device, m_window);
    }

    auto get_viewport() const -> FRect
    {
        return FRect {
            .left   = m_viewport.x,
            .top    = m_viewport.y,
            .right  = m_viewport.x + m_viewport.w,
            .bottom = m_viewport.y + m_viewport.h,
        };
    }

    [[nodiscard]] auto create_pipeline(entt::hashed_string const& name) -> std::shared_ptr<GraphicsPipeline>
    {
        if (!m_cache.contains<GraphicsPipeline>(name)) {
            m_cache.add(name, pipeline::create_by_name(m_device, m_window, m_asset_server, name));
        }

        return m_cache.get<GraphicsPipeline>(name);
    }

private:
    SDL_GPUViewport m_viewport = {};

    std::shared_ptr<AssetServer> m_asset_server;

    std::shared_ptr<SDL_Window>    m_window = nullptr;
    std::shared_ptr<SDL_GPUDevice> m_device = nullptr;

    RendererCache m_cache;
};

RendererService::RendererService(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader)
    : m_impl(std::make_unique<Impl>(asset_server, config_loader))
{
}

RendererService::~RendererService() = default;

void RendererService::bind_window(std::shared_ptr<SDL_Window> const& raw_window)
{
    assert(raw_window);
    m_impl->bind_window(raw_window);
}

void RendererService::unbind_window(std::shared_ptr<SDL_Window> const& raw_window)
{
    assert(raw_window);
    m_impl->unbind_window(raw_window);
}

auto RendererService::create_pipeline(entt::hashed_string const& name) -> std::shared_ptr<GraphicsPipeline>
{
    return m_impl->create_pipeline(name);
}

[[nodiscard]] auto RendererService::create_texture(entt::hashed_string const& name, TxFileHeader const& file_header)
    -> std::shared_ptr<GPUTexture>
{
    return m_impl->create_texture(name, file_header);
}

[[nodiscard]] auto RendererService::create_texture_target(uint32_t const width /*= 0*/, uint32_t const height /*= 0*/)
    -> std::unique_ptr<GPUTexture>
{
    return m_impl->create_texture_target(width, height);
}

auto RendererService::create_index_buffer(size_t const index_count) -> std::unique_ptr<GPUIndexBuffer>
{
    return m_impl->create_index_buffer(index_count);
}

auto RendererService::create_vertex_buffer(size_t const vertex_count, size_t const vertex_type_size) -> std::unique_ptr<GPUVertexBuffer>
{
    return m_impl->create_vertex_buffer(vertex_count, vertex_type_size);
}

auto RendererService::acquire_command_buffer() const -> std::unique_ptr<GPUCommandBuffer>
{
    return m_impl->acquire_command_buffer();
}

auto RendererService::get_viewport() const -> FRect
{
    return m_impl->get_viewport();
}
