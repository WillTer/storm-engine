#pragma once

#include <memory>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_pipeline.h>
#include <libs/renderer_next/vertex.h>

namespace storm
{

struct ShaderAsset;
class RendererSDL;

class PipelineSDL final: virtual public IPipeline
{
public:
    PipelineSDL(
        RendererSDL&                          renderer,
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info);

    ~PipelineSDL() override;

    void bind_to_render_pass() const override;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;
};

}  // namespace storm
