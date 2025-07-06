#include "colored_rect.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <libs/renderer_next/pipeline_names.h>

using namespace storm;
using namespace hlslpp;

using Vertex = shaders::colored_rect::VertexInput;

namespace
{

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

}  // namespace

ColoredRect::ColoredRect(storm::Color const& color)
{
    auto const& renderer = core->get<RendererService>();

    m_pipeline = renderer->create_pipeline(COLORED_RECT_PIPELINE);

    auto const [r, g, b, a] = color.normalize();

    auto const vertex_data = std::vector<Vertex> {
        Vertex {{0.0F, 0.0F}, float4 {r, g, b, a}},
        Vertex {{1.0F, 0.0F}, float4 {r, g, b, a}},
        Vertex {{1.0F, 1.0F}, float4 {r, g, b, a}},
        Vertex {{0.0F, 1.0F}, float4 {r, g, b, a}},
    };

    m_vertex_buffer = renderer->create_vertex_buffer(vertex_data);
    m_index_buffer  = renderer->create_index_buffer(SQUARE_INDICES);

    m_rect   = {0.0F, 0.0F, 1.0F, 1.0F};
    m_width  = 1;
    m_height = 1;

    m_is_color_dirty = false;
}

ColoredRect::~ColoredRect() = default;

void ColoredRect::update(GPUCopyPass const& copy_pass, uint64_t /*delta_time*/)
{
    if (m_is_color_dirty) {
        auto const [r, g, b, a] = m_color.normalize();

        std::vector const color_buffer = {
            // left-top
            decltype(Vertex::color)(r, g, b, a),
            // right-top
            decltype(Vertex::color)(r, g, b, a),
            // right-bottom
            decltype(Vertex::color)(r, g, b, a),
            // left-bottom
            decltype(Vertex::color)(r, g, b, a),
        };

        auto const progress_update_info =
            std::vector(4, BufferUpdateInfo {.offset = offsetof(Vertex, color), .size = sizeof(Vertex::color)});
        copy_pass.update_buffer(*m_vertex_buffer, progress_update_info, color_buffer, sizeof(Vertex));

        m_is_color_dirty = false;
    }
}

void ColoredRect::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);
    render_pass.draw(*m_index_buffer);
}

void ColoredRect::set_color(storm::Color const& color)
{
    m_color          = color;
    m_is_color_dirty = true;
}
