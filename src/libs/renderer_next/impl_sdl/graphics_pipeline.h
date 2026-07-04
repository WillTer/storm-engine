#pragma once

#include <memory>
#include <vector>

#include <shaders/info.h>
#include <shaders/vertex.h>

#include "sdl_fwd.h"

namespace storm
{

struct ShaderAsset;

class GraphicsPipeline final
{
public:
    enum class PrimitiveType { TriangleList, LineList };

    GraphicsPipeline(
        std::shared_ptr<SDL_GPUDevice> const&          device,
        std::shared_ptr<SDL_Window> const&             window,
        std::vector<shaders::VertexAttribute> const&   vertex_attributes,
        std::vector<shaders::VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                             vertex_shader_asset,
        shaders::Info const&                           vertex_shader_info,
        ShaderAsset const&                             fragment_shader_asset,
        shaders::Info const&                           fragment_shader_info,
        PrimitiveType                                  primitive_type);

    ~GraphicsPipeline();

    void bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const;

private:
    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;
};

}  // namespace storm
