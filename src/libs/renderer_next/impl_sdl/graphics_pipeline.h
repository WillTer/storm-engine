#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/vertex.h>

namespace storm
{

struct ShaderAsset;

struct ShaderInfo {
    uint32_t num_samplers;
    uint32_t num_storage_textures;
    uint32_t num_storage_buffers;
    uint32_t num_uniform_buffers;
};

class RendererService;

class GraphicsPipeline final
{
public:
    GraphicsPipeline(
        RendererService&                      renderer,
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info);

    ~GraphicsPipeline();

    void bind_to_render_pass() const;

private:
    RendererService& m_renderer;

    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;
};

}  // namespace storm
