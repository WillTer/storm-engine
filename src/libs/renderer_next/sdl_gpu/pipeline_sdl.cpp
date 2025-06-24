#include "pipeline_sdl.h"

#include <filesystem>

#include <libs/asset_server/shader_asset.h>
#include <libs/core/core.h>
#include <spdlog/spdlog.h>

#include "renderer_sdl.h"

using namespace storm;

namespace
{

std::shared_ptr<SDL_GPUShader> compile_shader(
    std::shared_ptr<SDL_GPUDevice> const& device, ShaderAsset const& asset, ShaderInfo const& info, SDL_GPUShaderStage const stage)
{
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
    if (asset.type == ShaderAssetType::SPIRV) {
        format = SDL_GPU_SHADERFORMAT_SPIRV;
    } else if (asset.type == ShaderAssetType::DXIL) {
        format = SDL_GPU_SHADERFORMAT_DXIL;
    }

    auto const shader_info = SDL_GPUShaderCreateInfo {
        .code_size    = asset.code.size(),                                 /**< The size in bytes of the code pointed to. */
        .code         = reinterpret_cast<Uint8 const*>(asset.code.data()), /**< A pointer to shader code. */
        .entrypoint   = "main", /**< A pointer to a null-terminated UTF-8 string specifying the entry point function name for the shader. */
        .format       = format, /**< The format of the shader code. */
        .stage        = stage,  /**< The stage the shader program corresponds to. */
        .num_samplers = info.num_samplers,                 /**< The number of samplers defined in the shader. */
        .num_storage_textures = info.num_storage_textures, /**< The number of storage textures defined in the shader. */
        .num_storage_buffers  = info.num_storage_buffers,  /**< The number of storage buffers defined in the shader. */
        .num_uniform_buffers  = info.num_uniform_buffers,  /**< The number of uniform buffers defined in the shader. */
        .props                = 0,                         /**< A properties ID for extensions. Should be 0 if no extensions are needed. */
    };

    return std::shared_ptr<SDL_GPUShader>(
        SDL_CreateGPUShader(device.get(), &shader_info), [device](SDL_GPUShader* p) { SDL_ReleaseGPUShader(device.get(), p); });
}

}  // namespace

template <>
auto storm::get_vertex_attributes<Position>() -> std::vector<SDL_GPUVertexAttribute>
{
    return {
        // position
        SDL_GPUVertexAttribute {
            .location    = 0,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = 0,
        },
    };
}

template <>
auto storm::get_vertex_attributes<PositionTexture>() -> std::vector<SDL_GPUVertexAttribute>
{
    return {
        // position
        SDL_GPUVertexAttribute {
            .location    = 0,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = 0,
        },
        // uv
        SDL_GPUVertexAttribute {
            .location    = 1,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset      = sizeof(PositionTexture::position),
        },
    };
}

template <>
auto storm::get_vertex_attributes<PositionTextureColor>() -> std::vector<SDL_GPUVertexAttribute>
{
    return {
        // position
        SDL_GPUVertexAttribute {
            .location    = 0,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = 0,
        },
        // uv
        SDL_GPUVertexAttribute {
            .location    = 1,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset      = sizeof(PositionTextureColor::position),
        },
        // diffuse
        SDL_GPUVertexAttribute {
            .location    = 2,
            .buffer_slot = 0,
            .format      = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset      = sizeof(PositionTextureColor::uv),
        },
    };
}

PipelineSDL::PipelineSDL(
    RendererSDL&                               renderer,
    std::shared_ptr<SDL_GPUCopyPass> const&    copy_pass,
    ShaderAsset const&                         vertex_shader_asset,
    ShaderInfo const&                          vertex_shader_info,
    ShaderAsset const&                         fragment_shader_asset,
    ShaderInfo const&                          fragment_shader_info,
    void const*                                vertex_data,
    uint32_t                                   vertex_count,
    uint32_t                                   vertex_size,
    std::vector<SDL_GPUVertexAttribute> const& vertex_attributes,
    std::vector<uint16_t> const&               indices)
    : m_renderer(renderer)
    , m_index_count(static_cast<uint32_t>(indices.size()))
{
    auto const device = m_renderer.get_device();

    auto const vertex_shader = compile_shader(device, vertex_shader_asset, vertex_shader_info, SDL_GPU_SHADERSTAGE_VERTEX);
    if (!vertex_shader) { throw std::runtime_error(std::format("Failed to compile vertex shader: {}", SDL_GetError())); }

    auto const fragment_shader = compile_shader(device, fragment_shader_asset, fragment_shader_info, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!fragment_shader) { throw std::runtime_error(std::format("Failed to compile fragment (pixel) shader: {}", SDL_GetError())); }

    std::array const descriptions = {
        SDL_GPUColorTargetDescription {
            .format      = m_renderer.get_spawchain_texture_format(),
            .blend_state = {},
        },
    };

    auto target_info                      = SDL_GPUGraphicsPipelineTargetInfo {};
    target_info.color_target_descriptions = descriptions.data();
    target_info.num_color_targets         = static_cast<Uint32>(descriptions.size());

    std::array const buffer_descriptions = {
        SDL_GPUVertexBufferDescription {
            .slot               = 0,
            .pitch              = vertex_size,
            .input_rate         = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0,
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
        SDL_CreateGPUGraphicsPipeline(device.get(), &pipeline_create_info),
        [device](SDL_GPUGraphicsPipeline* p) { SDL_ReleaseGPUGraphicsPipeline(device.get(), p); });

    if (!m_pipeline) { throw std::runtime_error(std::format("Failed to create GPU GraphicsPipeline: {}", SDL_GetError())); }

    uint32_t const vertex_data_size          = sizeof(PositionTexture) * vertex_count;
    auto           vertex_buffer_create_info = SDL_GPUBufferCreateInfo {};
    vertex_buffer_create_info.usage          = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer_create_info.size           = vertex_data_size;

    uint32_t const index_data_size          = sizeof(indices[0]) * m_index_count;
    auto           index_buffer_create_info = SDL_GPUBufferCreateInfo {};
    index_buffer_create_info.usage          = SDL_GPU_BUFFERUSAGE_INDEX;
    index_buffer_create_info.size           = index_data_size;

    m_vertex_buffer =
        std::shared_ptr<SDL_GPUBuffer>(SDL_CreateGPUBuffer(device.get(), &vertex_buffer_create_info), [device](SDL_GPUBuffer* p) {
            SDL_ReleaseGPUBuffer(device.get(), p);
        });
    if (!m_vertex_buffer) { throw std::runtime_error(std::format("Failed to create vertex buffer: {}", SDL_GetError())); }

    m_index_buffer =
        std::shared_ptr<SDL_GPUBuffer>(SDL_CreateGPUBuffer(device.get(), &index_buffer_create_info), [device](SDL_GPUBuffer* p) {
            SDL_ReleaseGPUBuffer(device.get(), p);
        });
    if (!m_index_buffer) { throw std::runtime_error(std::format("Failed to create index buffer: {}", SDL_GetError())); }

    auto vertex_transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    vertex_transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    vertex_transfer_buffer_create_info.size  = vertex_data_size + index_data_size;

    auto const vertex_transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &vertex_transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!vertex_transfer_buffer) { throw std::runtime_error(std::format("Failed to create vertex transfer buffer: {}", SDL_GetError())); }

    char* const vertex_transfer_data = static_cast<char*>(SDL_MapGPUTransferBuffer(device.get(), vertex_transfer_buffer.get(), false));
    std::memcpy(vertex_transfer_data, vertex_data, vertex_data_size);
    std::memcpy(vertex_transfer_data + vertex_data_size, indices.data(), index_data_size);
    SDL_UnmapGPUTransferBuffer(device.get(), vertex_transfer_buffer.get());

    auto const vertex_transfer_location = SDL_GPUTransferBufferLocation {
        .transfer_buffer = vertex_transfer_buffer.get(),
        .offset          = 0,
    };
    auto const vertex_buffer_region = SDL_GPUBufferRegion {
        .buffer = m_vertex_buffer.get(),
        .offset = 0,
        .size   = vertex_data_size,
    };
    SDL_UploadToGPUBuffer(copy_pass.get(), &vertex_transfer_location, &vertex_buffer_region, false);

    auto const index_transfer_location = SDL_GPUTransferBufferLocation {
        .transfer_buffer = vertex_transfer_buffer.get(),
        .offset          = vertex_data_size,
    };
    auto const index_buffer_region = SDL_GPUBufferRegion {
        .buffer = m_index_buffer.get(),
        .offset = 0,
        .size   = index_data_size,
    };
    SDL_UploadToGPUBuffer(copy_pass.get(), &index_transfer_location, &index_buffer_region, false);
}

PipelineSDL::~PipelineSDL() = default;

void PipelineSDL::present() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    if (!render_pass) {
        spdlog::error("No active render pass to bind");
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

    SDL_DrawGPUIndexedPrimitives(render_pass.get(), m_index_count, 1, 0, 0, 0);
}
