#pragma once

#include <memory>
#include <vector>

#include <libs/renderer_next/vertex.h>

#include "sdl_fwd.h"

namespace storm
{

struct ShaderAsset;

struct ShaderInfo {
    uint32_t num_samplers;
    uint32_t num_storage_textures;
    uint32_t num_storage_buffers;
    uint32_t num_uniform_buffers;
};

class GraphicsPipeline final
{
public:
    GraphicsPipeline(
        std::shared_ptr<SDL_GPUDevice> const& device,
        std::shared_ptr<SDL_Window> const&    window,
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info);

    ~GraphicsPipeline();

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;

private:
    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;
};

}  // namespace storm
