#include "renderer_sdl.h"

#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>

#include <libs/asset_server/asset_server.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/config/main_config.h>
#include <libs/core/core.h>
#include <libs/window/sdl_window.hpp>
#include <spdlog/spdlog.h>

#include "index_buffer_sdl.h"
#include "pipeline_sdl.h"
#include "texture_sdl.h"
#include "vertex_buffer_sdl.h"

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

RendererSDL::RendererSDL(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader)
{
    assert(asset_server);
    assert(config_loader);

    auto const device_info = main_config::device_info(*config_loader);

    m_backend = device_info.backend;
    if (!BACKEND_SHADER_EXT.contains(m_backend)) {
        spdlog::info("Unknown backend value in [device] settings: \"{}\", fallback to \"{}\"", m_backend, DEFAULT_BACKEND);
        m_backend = DEFAULT_BACKEND;
    }

    m_device = std::shared_ptr<SDL_GPUDevice>(
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV, IS_DEBUG_MODE, m_backend.c_str()),
        &SDL_DestroyGPUDevice);

    if (!m_device) { throw std::runtime_error(std::format("Failed to create GPU device: {}", SDL_GetError())); }

    asset_server->set_asset_ext<ShaderAsset>(BACKEND_SHADER_EXT.at(m_backend));

    m_viewport.min_depth = 0.0F;
    m_viewport.max_depth = 1.0F;
}

RendererSDL::~RendererSDL() = default;

void RendererSDL::bind_window(std::any const& window_raw)
{
    try {
        m_window = std::any_cast<SDL_Window*>(window_raw);
    } catch (std::bad_any_cast const&) {
        throw std::runtime_error("Only SDL window is supported for SDL_GPU API");
    }

    if (!SDL_ClaimWindowForGPUDevice(m_device.get(), m_window)) {
        throw std::runtime_error(std::format("Can't claim window for device: {}", SDL_GetError()));
    }

    int width  = 0;
    int height = 0;
    SDL_GetWindowSize(m_window, &width, &height);

    m_viewport.w = static_cast<float>(width);
    m_viewport.h = static_cast<float>(height);
}

void RendererSDL::unbind_window(std::any const& window_raw)
try {
    if (std::any_cast<SDL_Window*>(window_raw) != m_window) {
        spdlog::warn("Trying to unbind wrong window from renderer");
        return;
    }

    SDL_ReleaseWindowFromGPUDevice(m_device.get(), m_window);
    m_window = nullptr;
} catch (std::bad_any_cast const&) {
    throw std::runtime_error("Only SDL window is supported for SDL_GPU API");
}

std::unique_ptr<ITexture> RendererSDL::load_texture(TextureAsset const& asset)
{
    auto const cmd_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(m_device.get()), &SDL_SubmitGPUCommandBuffer);
    auto const copy_pass  = std::shared_ptr<SDL_GPUCopyPass>(SDL_BeginGPUCopyPass(cmd_buffer.get()), &SDL_EndGPUCopyPass);
    return std::make_unique<TextureSDL>(*this, copy_pass, asset);
}

std::unique_ptr<ITextureTarget> RendererSDL::create_texture_target()
{
    return std::make_unique<TextureSDL>(
        *this,
        static_cast<uint32_t>(m_viewport.w),
        static_cast<uint32_t>(m_viewport.h),
        1,
        SDL_GetGPUSwapchainTextureFormat(m_device.get(), m_window),
        SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET);
}

ITextureTarget& RendererSDL::get_default_texture_target()
{
    if (!m_default_texture_target) { m_default_texture_target = create_texture_target(); }

    return *m_default_texture_target;
}

std::unique_ptr<IPipeline> RendererSDL::create_pipeline(
    std::vector<VertexAttribute> const&   vertex_attributes,
    std::vector<VertexDescription> const& vertex_descriptions,
    ShaderAsset const&                    vertex_shader_asset,
    ShaderInfo const&                     vertex_shader_info,
    ShaderAsset const&                    fragment_shader_asset,
    ShaderInfo const&                     fragment_shader_info)
{
    return std::make_unique<PipelineSDL>(
        *this,
        vertex_attributes,
        vertex_descriptions,
        vertex_shader_asset,
        vertex_shader_info,
        fragment_shader_asset,
        fragment_shader_info);
}

std::unique_ptr<IIndexBuffer> RendererSDL::load_index_buffer(std::vector<uint16_t> const& buffer)
{
    auto const cmd_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(m_device.get()), &SDL_SubmitGPUCommandBuffer);
    auto const copy_pass  = std::shared_ptr<SDL_GPUCopyPass>(SDL_BeginGPUCopyPass(cmd_buffer.get()), &SDL_EndGPUCopyPass);
    return std::make_unique<IndexBufferSDL>(*this, copy_pass, buffer);
}

std::unique_ptr<IBuffer> RendererSDL::load_vertex_buffer(void const* data, uint32_t const data_size)
{
    auto const cmd_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(m_device.get()), &SDL_SubmitGPUCommandBuffer);
    auto const copy_pass  = std::shared_ptr<SDL_GPUCopyPass>(SDL_BeginGPUCopyPass(cmd_buffer.get()), &SDL_EndGPUCopyPass);
    return std::make_unique<VertexBufferSDL>(*this, copy_pass, data, data_size);
}

void RendererSDL::start_frame()
{
    m_current_command_buffer =
        std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(m_device.get()), &SDL_SubmitGPUCommandBuffer);
    if (!m_current_command_buffer) { spdlog::error("Acquire GPU command buffer failed: {}", SDL_GetError()); }

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_current_command_buffer.get(), m_window, &m_swapchain_texture, nullptr, nullptr)
        || m_swapchain_texture == nullptr) {
        spdlog::error("Acquire GPU swapchain texture failed: {}", SDL_GetError());
        return;
    }

    auto color_target_info        = SDL_GPUColorTargetInfo {};
    color_target_info.texture     = m_swapchain_texture;
    color_target_info.clear_color = {0.0F, 0.0F, 0.0F, 1.0F};  // Black
    color_target_info.load_op     = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op    = SDL_GPU_STOREOP_STORE;

    // Clear screen pass
    auto render_pass = std::shared_ptr<SDL_GPURenderPass>(
        SDL_BeginGPURenderPass(m_current_command_buffer.get(), &color_target_info, 1, nullptr), &SDL_EndGPURenderPass);
    if (!render_pass) { spdlog::error("Begin GPU render pass failed: {}", SDL_GetError()); }
}

void RendererSDL::end_frame()
{
    m_swapchain_texture = nullptr;
    m_current_command_buffer.reset();
}

void RendererSDL::start_render_pass()
{
    if (!m_current_command_buffer) {
        spdlog::error("No active command buffer on start_pass");
        return;
    }

    if (m_current_render_pass) {
        spdlog::error("Render pass already started");
        return;
    }

    auto color_target_info     = SDL_GPUColorTargetInfo {};
    color_target_info.texture  = m_swapchain_texture;
    color_target_info.load_op  = SDL_GPU_LOADOP_LOAD;  // Load texture state from previous pass
    color_target_info.store_op = SDL_GPU_STOREOP_STORE;

    m_current_render_pass = std::shared_ptr<SDL_GPURenderPass>(
        SDL_BeginGPURenderPass(m_current_command_buffer.get(), &color_target_info, 1, nullptr), &SDL_EndGPURenderPass);
    if (!m_current_render_pass) { spdlog::error("Begin GPU render pass failed: {}", SDL_GetError()); }

    SDL_SetGPUViewport(m_current_render_pass.get(), &m_viewport);
}

void RendererSDL::end_render_pass()
{
    m_current_render_pass.reset();
}

FRect RendererSDL::get_viewport() const
{
    return FRect {
        .left   = m_viewport.x,
        .top    = m_viewport.y,
        .right  = m_viewport.x + m_viewport.w,
        .bottom = m_viewport.y + m_viewport.h,
    };
}

void RendererSDL::push_vertex_uniform_data(uint32_t const slot, void const* data, uint32_t const data_size)
{
    assert(m_current_command_buffer);
    SDL_PushGPUVertexUniformData(m_current_command_buffer.get(), slot, data, data_size);
}

void RendererSDL::push_fragment_uniform_data(uint32_t slot, void const* data, uint32_t data_size)
{
    assert(m_current_command_buffer);
    SDL_PushGPUFragmentUniformData(m_current_command_buffer.get(), slot, data, data_size);
}

SDL_GPUTextureFormat RendererSDL::get_spawchain_texture_format() const
{
    if (m_window == nullptr) {
        spdlog::error("Trying to get swapchain texture format while window is not claimed");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    return SDL_GetGPUSwapchainTextureFormat(m_device.get(), m_window);
}

std::shared_ptr<SDL_GPUDevice> const& RendererSDL::get_device() const
{
    return m_device;
}

std::shared_ptr<SDL_GPUCommandBuffer> const& RendererSDL::get_current_command_buffer() const
{
    return m_current_command_buffer;
}

std::shared_ptr<SDL_GPURenderPass> const& RendererSDL::get_current_render_pass() const
{
    return m_current_render_pass;
}

void RendererSDL::start_render_pass(std::shared_ptr<SDL_GPURenderPass> const& texture_render_pass)
{
    assert(texture_render_pass);
    m_current_render_pass = texture_render_pass;
    SDL_SetGPUViewport(m_current_render_pass.get(), &m_viewport);
}
