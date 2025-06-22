#include "renderer_sdl.h"

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <libs/asset_server/asset_server.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/asset_server/texture_asset.h>
#include <libs/core/core.h>
#include <libs/window/sdl_window.hpp>

using namespace storm;

namespace
{
#ifdef _DEBUG
constexpr bool IS_DEBUG_MODE = true;
#else
constexpr bool IS_DEBUG_MODE = false;
#endif

#ifdef _WIN32
constexpr char BACKEND[]    = "direct3d12";
constexpr char SHADER_EXT[] = "bin";
#else
constexpr char BACKEND[]    = "vulkan";
constexpr char SHADER_EXT[] = "spv";
#endif

struct PositionColor {
    std::array<float, 3> position;
    std::array<float, 3> color;
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
    m_device = std::shared_ptr<SDL_GPUDevice>(
        SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_SPIRV, IS_DEBUG_MODE, BACKEND), &SDL_DestroyGPUDevice);

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
    auto const  shader_load  = asset_server->get_loader<ShaderAsset, AssetServer::NoCache>(SHADER_EXT);
    auto const  tex_load     = asset_server->get_loader<TextureAsset const&>();

    // Testing
    auto const vertex_shader = compile_shader(m_device, shader_load, "test_vs", ShaderStage::Vertex, 0, 0, 0, 0);
    if (!vertex_shader) { throw std::runtime_error(std::format("Failed to compile vertex shader: {}", SDL_GetError())); }

    auto const fragment_shader = compile_shader(m_device, shader_load, "test_ps", ShaderStage::Fragment, 0, 0, 0, 0);
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
            .pitch              = sizeof(PositionColor),
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
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = sizeof(PositionColor::position),
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

    auto const& texture = tex_load("loading/storm.tga.tx");

    auto texture_create_info                 = SDL_GPUTextureCreateInfo {};
    texture_create_info.type                 = SDL_GPU_TEXTURETYPE_2D;
    texture_create_info.format               = convert_tx_format(texture.header.format);
    texture_create_info.width                = texture.header.width;
    texture_create_info.height               = texture.header.height;
    texture_create_info.layer_count_or_depth = 1;
    texture_create_info.num_levels           = texture.header.mip_levels;
    texture_create_info.usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    m_texture =
        std::shared_ptr<SDL_GPUTexture>(SDL_CreateGPUTexture(m_device.get(), &texture_create_info), [device = m_device](SDL_GPUTexture* p) {
            SDL_ReleaseGPUTexture(device.get(), p);
        });
    if (!m_texture) { throw std::runtime_error(std::format("Failed to create texture: {}", SDL_GetError())); }
}
