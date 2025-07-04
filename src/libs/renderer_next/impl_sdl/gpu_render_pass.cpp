#include "gpu_render_pass.h"

#include <SDL3/SDL_gpu.h>
#include <spdlog/spdlog.h>

using namespace storm;

namespace
{

std::vector<SDL_GPUColorTargetInfo> convert_color_target_info(std::vector<ColorTargetInfo> const& infos)
{
    std::vector<SDL_GPUColorTargetInfo> result = {};
    std::transform(infos.begin(), infos.end(), std::back_inserter(result), [](ColorTargetInfo const& info) {
        return SDL_GPUColorTargetInfo {
            .texture               = info.target_texture,
            .mip_level             = 0,
            .layer_or_depth_plane  = 0,
            .clear_color           = {info.clear_color.r, info.clear_color.g, info.clear_color.b, info.clear_color.a},
            .load_op               = static_cast<SDL_GPULoadOp>(info.load_op),
            .store_op              = static_cast<SDL_GPUStoreOp>(info.store_op),
            .resolve_texture       = nullptr,  // TODO: No multisampling resolve for now
            .resolve_mip_level     = 0,
            .resolve_layer         = 0,
            .cycle                 = 0,
            .cycle_resolve_texture = 0,
            .padding1              = 0,
            .padding2              = 0,
        };
    });

    return result;
}

}  // namespace

GPURenderPass::GPURenderPass(
    std::shared_ptr<SDL_GPUCommandBuffer> const& cmd_buffer,
    std::vector<ColorTargetInfo> const&          color_targets,
    std::optional<storm::FRect> const&           viewport)
    : m_cmd_buffer(cmd_buffer)
{
    auto color_target_info = convert_color_target_info(color_targets);

    m_pass = std::shared_ptr<SDL_GPURenderPass>(
        SDL_BeginGPURenderPass(cmd_buffer.get(), color_target_info.data(), static_cast<uint32_t>(color_target_info.size()), nullptr),
        &SDL_EndGPURenderPass);
    if (!m_pass) {
        spdlog::error("Begin GPU render pass failed: {}", SDL_GetError());
        return;
    }

    if (viewport.has_value()) {
        auto const& vp = viewport.value();

        auto const viewport_native = SDL_GPUViewport {
            .x         = vp.left,
            .y         = vp.top,
            .w         = vp.width(),
            .h         = vp.height(),
            .min_depth = 0.0F,
            .max_depth = 1.0F,
        };

        SDL_SetGPUViewport(m_pass.get(), &viewport_native);
    }
}

GPURenderPass::~GPURenderPass() = default;

void GPURenderPass::push_vertex_uniform_data(uint32_t const slot, void const* data, size_t const data_size) const
{
    SDL_PushGPUVertexUniformData(m_cmd_buffer.get(), slot, data, static_cast<uint32_t>(data_size));
}

void GPURenderPass::push_fragment_uniform_data(uint32_t const slot, void const* data, size_t const data_size) const
{
    SDL_PushGPUFragmentUniformData(m_cmd_buffer.get(), slot, data, static_cast<uint32_t>(data_size));
}
