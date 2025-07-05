#include "graphics_pipeline.h"

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/core/core.h>
#include <spdlog/spdlog.h>

#include "renderer_sdl.h"

using namespace storm;

namespace
{

std::shared_ptr<SDL_GPUShader> compile_shader(
    std::shared_ptr<SDL_GPUDevice> const& device, ShaderAsset const& asset, shaders::Info const& info, SDL_GPUShaderStage const stage)
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

std::vector<SDL_GPUVertexAttribute> convert_attributes(std::vector<shaders::VertexAttribute> const& attributes)
{
    std::vector<SDL_GPUVertexAttribute> result;
    std::ranges::transform(attributes, std::back_inserter(result), [](shaders::VertexAttribute const& attr) {
        return SDL_GPUVertexAttribute {
            .location    = attr.location,
            .buffer_slot = attr.slot,
            .format      = static_cast<SDL_GPUVertexElementFormat>(attr.format),  // They should be compatible
            .offset      = attr.offset,
        };
    });

    return result;
}

std::vector<SDL_GPUVertexBufferDescription> convert_descriptions(std::vector<shaders::VertexDescription> const& descriptions)
{
    std::vector<SDL_GPUVertexBufferDescription> result;
    std::ranges::transform(descriptions, std::back_inserter(result), [](shaders::VertexDescription const& desc) {
        return SDL_GPUVertexBufferDescription {
            .slot               = desc.slot,
            .pitch              = desc.stride,
            .input_rate         = static_cast<SDL_GPUVertexInputRate>(desc.input_rate),  // They should be compatible
            .instance_step_rate = desc.instance_step_rate,
        };
    });

    return result;
}

}  // namespace

GraphicsPipeline::GraphicsPipeline(
    std::shared_ptr<SDL_GPUDevice> const&          device,
    std::shared_ptr<SDL_Window> const&             window,
    std::vector<shaders::VertexAttribute> const&   vertex_attributes,
    std::vector<shaders::VertexDescription> const& vertex_descriptions,
    ShaderAsset const&                             vertex_shader_asset,
    shaders::Info const&                           vertex_shader_info,
    ShaderAsset const&                             fragment_shader_asset,
    shaders::Info const&                           fragment_shader_info)
{
    auto const vertex_shader = compile_shader(device, vertex_shader_asset, vertex_shader_info, SDL_GPU_SHADERSTAGE_VERTEX);
    if (!vertex_shader) { throw std::runtime_error(std::format("Failed to compile vertex shader: {}", SDL_GetError())); }

    auto const fragment_shader = compile_shader(device, fragment_shader_asset, fragment_shader_info, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (!fragment_shader) { throw std::runtime_error(std::format("Failed to compile fragment (pixel) shader: {}", SDL_GetError())); }

    auto blend_state                  = SDL_GPUColorTargetBlendState {};
    blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
    blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    blend_state.color_blend_op        = SDL_GPU_BLENDOP_ADD;
    blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
    blend_state.alpha_blend_op        = SDL_GPU_BLENDOP_ADD;
    blend_state.enable_blend          = true;

    std::array const color_descriptions = {
        SDL_GPUColorTargetDescription {
            .format      = SDL_GetGPUSwapchainTextureFormat(device.get(), window.get()),
            .blend_state = blend_state,
        },
    };

    auto target_info                      = SDL_GPUGraphicsPipelineTargetInfo {};
    target_info.color_target_descriptions = color_descriptions.data();
    target_info.num_color_targets         = static_cast<Uint32>(color_descriptions.size());

    auto const attributes   = convert_attributes(vertex_attributes);
    auto const descriptions = convert_descriptions(vertex_descriptions);

    auto const vertex_input_state = SDL_GPUVertexInputState {
        .vertex_buffer_descriptions = descriptions.data(),
        .num_vertex_buffers         = static_cast<Uint32>(descriptions.size()),
        .vertex_attributes          = attributes.data(),
        .num_vertex_attributes      = static_cast<Uint32>(attributes.size()),
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
}

GraphicsPipeline::~GraphicsPipeline() = default;

void GraphicsPipeline::bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const
{
    assert(render_pass);
    SDL_BindGPUGraphicsPipeline(render_pass.get(), m_pipeline.get());
}
