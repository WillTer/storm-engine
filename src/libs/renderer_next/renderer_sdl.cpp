#include "renderer_sdl.h"

#include <array>
#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <libs/asset_server/asset_server.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/asset_server/texture_asset.h>
#include <libs/config/main_config.h>
#include <libs/core/core.h>
#include <libs/window/sdl_window.hpp>
#include <spdlog/spdlog.h>

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
    {"direct3d12", "bin"},
    {"vulkan", "spv"},
};
#else
constexpr std::string_view DEFAULT_BACKEND    = "vulkan";
auto const                 BACKEND_SHADER_EXT = std::unordered_map<std::string, std::string> {
    {"vulkan", "spv"},
};
#endif

struct PositionTexture {
    std::array<float, 3> position;
    std::array<float, 2> uv;
};

// TODO: move to another file
SDL_GPUShaderStage convert_shader_stage(ShaderStage stage)
{
    switch (stage) {
    case ShaderStage::Vertex: return SDL_GPU_SHADERSTAGE_VERTEX;
    case ShaderStage::Fragment: return SDL_GPU_SHADERSTAGE_FRAGMENT;
    default: throw std::runtime_error("Invalid shader stage");
    }
}

// TODO: move to another file
std::shared_ptr<SDL_GPUShader> compile_shader(
    std::shared_ptr<SDL_GPUDevice> const&                           device,
    std::function<ShaderAsset(std::filesystem::path const&)> const& shader_load,
    std::filesystem::path const&                                    path,
    ShaderStage const                                               stage,
    uint32_t const                                                  num_samplers,
    uint32_t const                                                  num_storage_textures,
    uint32_t const                                                  num_storage_buffers,
    uint32_t const                                                  num_uniform_buffers)
{
    auto const shader_asset = shader_load(path);

    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    if (shader_asset.type == ShaderAssetType::SPIRV) {
        format = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if (shader_asset.type == ShaderAssetType::DXIL) {
        format = SDL_GPU_SHADERFORMAT_DXIL;
    }

    auto const shader_info = SDL_GPUShaderCreateInfo {
        .code_size    = shader_asset.code.size(), /**< The size in bytes of the code pointed to. */
        .code         = shader_asset.code.data(), /**< A pointer to shader code. */
        .entrypoint   = "main", /**< A pointer to a null-terminated UTF-8 string specifying the entry point function name for the shader. */
        .format       = format, /**< The format of the shader code. */
        .stage        = convert_shader_stage(stage),  /**< The stage the shader program corresponds to. */
        .num_samplers = num_samplers,                 /**< The number of samplers defined in the shader. */
        .num_storage_textures = num_storage_textures, /**< The number of storage textures defined in the shader. */
        .num_storage_buffers  = num_storage_buffers,  /**< The number of storage buffers defined in the shader. */
        .num_uniform_buffers  = num_uniform_buffers,  /**< The number of uniform buffers defined in the shader. */
        .props                = 0,                    /**< A properties ID for extensions. Should be 0 if no extensions are needed. */
    };

    return std::shared_ptr<SDL_GPUShader>(
        SDL_CreateGPUShader(device.get(), &shader_info), [device](SDL_GPUShader* p) { SDL_ReleaseGPUShader(device.get(), p); });
}

// TODO: move to another file
SDL_GPUTextureFormat convert_tx_format(TxFormat const format)
{
    switch (format) {
    case TxFormat::A8R8G8B8: return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
    case TxFormat::X8R8G8B8: return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
    case TxFormat::R5G6B5: return SDL_GPU_TEXTUREFORMAT_B5G6R5_UNORM;
    case TxFormat::A1R5G5B5: return SDL_GPU_TEXTUREFORMAT_B5G5R5A1_UNORM;
    case TxFormat::A4R4G4B4: return SDL_GPU_TEXTUREFORMAT_B4G4R4A4_UNORM;
    case TxFormat::L8: return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
    case TxFormat::V8U8: return SDL_GPU_TEXTUREFORMAT_R8G8_SNORM;

    case TxFormat::DXT1: return SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM;
    case TxFormat::DXT2: return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
    case TxFormat::DXT3: return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
    case TxFormat::DXT4: return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
    case TxFormat::DXT5: return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
    default: break;
    }

    return SDL_GPU_TEXTUREFORMAT_INVALID;
}

}  // namespace

RendererSDL::RendererSDL()
{
    auto const device_info = storm::main_config::device_info();

    m_backend = device_info.backend;
    if (!BACKEND_SHADER_EXT.contains(m_backend)) {
        spdlog::info("Unknown backend value in [device] settings: \"{}\", fallback to \"{}\"", m_backend, DEFAULT_BACKEND);
        m_backend = DEFAULT_BACKEND;
    }

    m_device = std::shared_ptr<SDL_GPUDevice>(
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV, IS_DEBUG_MODE, m_backend.c_str()),
        &SDL_DestroyGPUDevice);

    if (!m_device) { throw std::runtime_error(std::format("Failed to create GPU device: {}", SDL_GetError())); }
}

RendererSDL::~RendererSDL() = default;

void RendererSDL::bind_window(InternalWindowType const& window)
{
    if (!std::holds_alternative<SDL_Window*>(window)) { throw std::runtime_error("Only SDL window is supported for SDL_GPU API"); }

    auto* sdl_window = std::get<SDL_Window*>(window);
    if (!SDL_ClaimWindowForGPUDevice(m_device.get(), sdl_window)) {
        throw std::runtime_error(std::format("Can't claim window for device: {}", SDL_GetError()));
    }

    m_window = sdl_window;
}

void RendererSDL::unbind_window(InternalWindowType const& window)
{
    if (!std::holds_alternative<SDL_Window*>(window)) { throw std::runtime_error("Only SDL window is supported for SDL_GPU API"); }

    auto* sdl_window = std::get<SDL_Window*>(window);
    SDL_ReleaseWindowFromGPUDevice(m_device.get(), sdl_window);

    m_window = nullptr;
}

void RendererSDL::init()
{
    auto const& asset_server = core->get<AssetServer>();
    auto const  shader_load  = asset_server->get_loader<ShaderAsset, AssetServer::NoCache>(BACKEND_SHADER_EXT.at(m_backend));

    // Testing
    auto const vertex_shader = compile_shader(m_device, shader_load, "test_vs", ShaderStage::Vertex, 0, 0, 0, 0);
    if (!vertex_shader) { throw std::runtime_error(std::format("Failed to compile vertex shader: {}", SDL_GetError())); }

    auto const fragment_shader = compile_shader(m_device, shader_load, "test_fs", ShaderStage::Fragment, 1, 0, 0, 0);
    if (!fragment_shader) { throw std::runtime_error(std::format("Failed to compile fragment (pixel) shader: {}", SDL_GetError())); }

    std::array const descriptions = {
        SDL_GPUColorTargetDescription {
            .format      = SDL_GetGPUSwapchainTextureFormat(m_device.get(), m_window),
            .blend_state = {},
        },
    };

    auto target_info                      = SDL_GPUGraphicsPipelineTargetInfo {};
    target_info.color_target_descriptions = descriptions.data();
    target_info.num_color_targets         = static_cast<Uint32>(descriptions.size());

    constexpr std::array buffer_descriptions = {
        SDL_GPUVertexBufferDescription {
            .slot               = 0,
            .pitch              = sizeof(PositionTexture),
            .input_rate         = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
        },
    };

    constexpr std::array vertex_attributes = {
        SDL_GPUVertexAttribute {
            .location    = 0,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = 0,
        },
        SDL_GPUVertexAttribute {
            .location    = 1,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset      = sizeof(PositionTexture::position),
        },
    };

    auto const vertex_input_state = SDL_GPUVertexInputState {
        .vertex_buffer_descriptions = buffer_descriptions.data(),
        .num_vertex_buffers         = static_cast<Uint32>(buffer_descriptions.size()),
        .vertex_attributes          = vertex_attributes.data(),
        .num_vertex_attributes      = static_cast<Uint32>(vertex_attributes.size()),
    };

    auto pipeline_create_info                       = SDL_GPUGraphicsPipelineCreateInfo {};
    pipeline_create_info.target_info                = target_info;
    pipeline_create_info.vertex_input_state         = vertex_input_state;
    pipeline_create_info.primitive_type             = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipeline_create_info.vertex_shader              = vertex_shader.get();
    pipeline_create_info.fragment_shader            = fragment_shader.get();
    pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    m_pipeline = std::shared_ptr<SDL_GPUGraphicsPipeline>(
        SDL_CreateGPUGraphicsPipeline(m_device.get(), &pipeline_create_info),
        [device = m_device](SDL_GPUGraphicsPipeline* p) { SDL_ReleaseGPUGraphicsPipeline(device.get(), p); });

    if (!m_pipeline) { throw std::runtime_error(std::format("Failed to create GPU GraphicsPipeline: {}", SDL_GetError())); }

    auto sampler_create_info              = SDL_GPUSamplerCreateInfo {};
    sampler_create_info.min_filter        = SDL_GPU_FILTER_LINEAR;
    sampler_create_info.mag_filter        = SDL_GPU_FILTER_LINEAR;
    sampler_create_info.mipmap_mode       = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    sampler_create_info.address_mode_u    = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_create_info.address_mode_v    = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_create_info.address_mode_w    = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_create_info.enable_anisotropy = true;
    sampler_create_info.max_anisotropy    = 16.0F;

    m_sampler =
        std::shared_ptr<SDL_GPUSampler>(SDL_CreateGPUSampler(m_device.get(), &sampler_create_info), [device = m_device](SDL_GPUSampler* p) {
            SDL_ReleaseGPUSampler(device.get(), p);
        });

    if (!m_sampler) { throw std::runtime_error(std::format("Failed to create GPU Sampler: {}", SDL_GetError())); }

    auto const& texture = asset_server->get_texture("loading/storm.tga.tx");

    auto texture_create_info                 = SDL_GPUTextureCreateInfo {};
    texture_create_info.type                 = SDL_GPU_TEXTURETYPE_2D;
    texture_create_info.format               = convert_tx_format(texture.header.format);
    texture_create_info.usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texture_create_info.width                = texture.header.width;
    texture_create_info.height               = texture.header.height;
    texture_create_info.layer_count_or_depth = 1;
    texture_create_info.num_levels           = texture.header.mip_levels;

    m_texture =
        std::shared_ptr<SDL_GPUTexture>(SDL_CreateGPUTexture(m_device.get(), &texture_create_info), [device = m_device](SDL_GPUTexture* p) {
            SDL_ReleaseGPUTexture(device.get(), p);
        });
    if (!m_texture) { throw std::runtime_error(std::format("Failed to create texture: {}", SDL_GetError())); }

    auto texture_transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    texture_transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    texture_transfer_buffer_create_info.size  = static_cast<Uint32>(texture.data.size());

    auto const texture_transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(m_device.get(), &texture_transfer_buffer_create_info),
        [device = m_device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!texture_transfer_buffer) {
        throw std::runtime_error(std::format("Failed to create transfer buffer for texture: {}", SDL_GetError()));
    }

    char* const transfer_data = static_cast<char*>(SDL_MapGPUTransferBuffer(m_device.get(), texture_transfer_buffer.get(), false));
    std::memcpy(transfer_data, texture.data.data(), texture.data.size());
    SDL_UnmapGPUTransferBuffer(m_device.get(), texture_transfer_buffer.get());

    auto vertex_buffer_create_info  = SDL_GPUBufferCreateInfo {};
    vertex_buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer_create_info.size  = sizeof(PositionTexture) * 4;

    auto index_buffer_create_info  = SDL_GPUBufferCreateInfo {};
    index_buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer_create_info.size  = sizeof(Uint16) * 6;

    m_vertex_buffer = std::shared_ptr<SDL_GPUBuffer>(
        SDL_CreateGPUBuffer(m_device.get(), &vertex_buffer_create_info),
        [device = m_device](SDL_GPUBuffer* p) { SDL_ReleaseGPUBuffer(device.get(), p); });
    if (!m_vertex_buffer) { throw std::runtime_error(std::format("Failed to create vertex buffer: {}", SDL_GetError())); }

    m_index_buffer = std::shared_ptr<SDL_GPUBuffer>(
        SDL_CreateGPUBuffer(m_device.get(), &index_buffer_create_info),
        [device = m_device](SDL_GPUBuffer* p) { SDL_ReleaseGPUBuffer(device.get(), p); });
    if (!m_index_buffer) { throw std::runtime_error(std::format("Failed to create index buffer: {}", SDL_GetError())); }

    auto vertex_transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    vertex_transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    vertex_transfer_buffer_create_info.size  = sizeof(PositionTexture) * 4 + sizeof(Uint16) * 6;

    auto const vertex_transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(m_device.get(), &vertex_transfer_buffer_create_info),
        [device = m_device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!vertex_transfer_buffer) { throw std::runtime_error(std::format("Failed to create vertex transfer buffer: {}", SDL_GetError())); }

    constexpr std::array vertex_data = {
        PositionTexture {{-1.0F, 1.0F, 0.0F}, {0.0F, 0.0F}},
        PositionTexture {{1.0F, 1.0F, 0.0F}, {1.0F, 0.0F}},
        PositionTexture {{1.0F, -1.0F, 0.0F}, {1.0F, 1.0F}},
        PositionTexture {{-1.0F, -1.0F, 0.0F}, {0.0F, 1.0F}},
    };

    constexpr std::array<Uint16, 6> index_data = {0, 1, 2, 0, 2, 3};

    char* const vertex_transfer_data = static_cast<char*>(SDL_MapGPUTransferBuffer(m_device.get(), vertex_transfer_buffer.get(), false));
    std::memcpy(vertex_transfer_data, vertex_data.data(), sizeof(vertex_data));
    std::memcpy(vertex_transfer_data + sizeof(vertex_data), index_data.data(), sizeof(index_data));
    SDL_UnmapGPUTransferBuffer(m_device.get(), vertex_transfer_buffer.get());

    {
        auto const upload_cmd_buffer =
            std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(m_device.get()), &SDL_SubmitGPUCommandBuffer);
        auto const copy_pass = std::shared_ptr<SDL_GPUCopyPass>(SDL_BeginGPUCopyPass(upload_cmd_buffer.get()), &SDL_EndGPUCopyPass);

        auto const vertex_transfer_location = SDL_GPUTransferBufferLocation {
            .transfer_buffer = vertex_transfer_buffer.get(),
            .offset          = 0,
        };
        auto const vertex_buffer_region = SDL_GPUBufferRegion {
            .buffer = m_vertex_buffer.get(),
            .offset = 0,
            .size   = sizeof(vertex_data),
        };
        SDL_UploadToGPUBuffer(copy_pass.get(), &vertex_transfer_location, &vertex_buffer_region, false);

        auto const index_transfer_location = SDL_GPUTransferBufferLocation {
            .transfer_buffer = vertex_transfer_buffer.get(),
            .offset          = sizeof(vertex_data),
        };
        auto const index_buffer_region = SDL_GPUBufferRegion {
            .buffer = m_index_buffer.get(),
            .offset = 0,
            .size   = sizeof(index_data),
        };
        SDL_UploadToGPUBuffer(copy_pass.get(), &index_transfer_location, &index_buffer_region, false);

        auto const tex_transfer_location = SDL_GPUTextureTransferInfo {
            .transfer_buffer = texture_transfer_buffer.get(),
            .offset          = 0,
            .pixels_per_row  = 0,
            .rows_per_layer  = 0,
        };
        auto const tex_buffer_region = SDL_GPUTextureRegion {
            .texture   = m_texture.get(),
            .mip_level = 0,
            .layer     = 0,
            .x         = 0,
            .y         = 0,
            .z         = 0,
            .w         = texture.header.width,
            .h         = texture.header.height,
            .d         = 1,
        };
        SDL_UploadToGPUTexture(copy_pass.get(), &tex_transfer_location, &tex_buffer_region, false);
    }
}

void RendererSDL::draw()
{
    auto const cmd_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(m_device.get()), &SDL_SubmitGPUCommandBuffer);
    if (!cmd_buffer) {
        spdlog::error("Acquire GPU command buffer failed: {}", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain_texture = nullptr;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd_buffer.get(), m_window, &swapchain_texture, nullptr, nullptr)
        || swapchain_texture == nullptr) {
        spdlog::error("Acquire GPU swapchain texture failed: {}", SDL_GetError());
        return;
    }

    auto color_target_info        = SDL_GPUColorTargetInfo {};
    color_target_info.texture     = swapchain_texture;
    color_target_info.clear_color = {0.0F, 0.0F, 0.0F, 1.0F};  // Black
    color_target_info.load_op     = SDL_GPU_LOADOP_CLEAR;
    color_target_info.store_op    = SDL_GPU_STOREOP_STORE;

    auto const render_pass =
        std::shared_ptr<SDL_GPURenderPass>(SDL_BeginGPURenderPass(cmd_buffer.get(), &color_target_info, 1, nullptr), &SDL_EndGPURenderPass);
    if (!render_pass) {
        spdlog::error("Begin GPU render pass failed: {}", SDL_GetError());
        return;
    }

    SDL_BindGPUGraphicsPipeline(render_pass.get(), m_pipeline.get());

    auto vertex_binding   = SDL_GPUBufferBinding {};
    vertex_binding.buffer = m_vertex_buffer.get();
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(render_pass.get(), 0, &vertex_binding, 1);

    auto index_binding   = SDL_GPUBufferBinding {};
    index_binding.buffer = m_index_buffer.get();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(render_pass.get(), &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

    auto texture_binding    = SDL_GPUTextureSamplerBinding {};
    texture_binding.sampler = m_sampler.get();
    texture_binding.texture = m_texture.get();
    SDL_BindGPUFragmentSamplers(render_pass.get(), 0, &texture_binding, 1);

    SDL_DrawGPUIndexedPrimitives(render_pass.get(), 6, 1, 0, 0, 0);
}
