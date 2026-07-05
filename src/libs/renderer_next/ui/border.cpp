#include "border.h"

#include <cassert>

#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

using namespace storm;
using namespace hlslpp;

using Vertex = shaders::rectangle::VertexInput;

namespace
{

auto const SQUARE_LINES_INDICES = std::vector<uint32_t> {0, 1, 1, 2, 2, 3, 3, 0};

}  // namespace

Border::Border(storm::Color const& color, std::optional<std::string> const& technique /*= std::nullopt*/)
{
    auto const& renderer = core->get<RendererService>();

    if (technique.has_value()) {
        m_pipeline = renderer->create_pipeline_from_technique<Vertex>("ui/rectangle", technique.value());
    } else {
        m_pipeline = renderer->create_pipeline<Vertex>("ui/rectangle", "ui/color_only");
    }

    auto const [r, g, b, a] = color.normalize();

    auto const vertex_data = std::vector<Vertex> {
        Vertex {{0.0F, 0.0F}, float4 {r, g, b, a}},
        Vertex {{1.0F, 0.0F}, float4 {r, g, b, a}},
        Vertex {{1.0F, 1.0F}, float4 {r, g, b, a}},
        Vertex {{0.0F, 1.0F}, float4 {r, g, b, a}},
    };

    m_vertex_buffer = renderer->create_vertex_buffer(vertex_data);
    m_index_buffer  = renderer->create_index_buffer(SQUARE_LINES_INDICES);

    m_rect   = {0.0F, 0.0F, 1.0F, 1.0F};
    m_width  = 1;
    m_height = 1;

    m_is_color_dirty = false;
}

Border::~Border() = default;

void Border::update(GPUCopyPass const& copy_pass, uint64_t /*delta_time*/)
{
    if (m_is_color_dirty) {
        std::vector<decltype(Vertex::color)> color_buffer = {};
        std::transform(m_color.begin(), m_color.end(), std::back_inserter(color_buffer), [](storm::Color const& color) {
            auto const [r, g, b, a] = color.normalize();
            return decltype(Vertex::color)(r, g, b, a);
        });

        auto const progress_update_info =
            std::vector(4, BufferUpdateInfo {.offset = offsetof(Vertex, color), .size = sizeof(Vertex::color)});
        copy_pass.update_buffer(*m_vertex_buffer, progress_update_info, color_buffer, sizeof(Vertex));

        m_is_color_dirty = false;
    }
}

void Border::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);
    render_pass.draw(*m_index_buffer);
}

void Border::set_color(storm::Color const& color)
{
    for (size_t i = 0; i < m_color.size(); ++i) {
        m_color[i] = color;
    }

    m_is_color_dirty = true;
}

void Border::set_vertex_color(size_t index, storm::Color const& color)
{
    // Out of bounds
    if (index >= m_color.size()) {
        return;
    }

    m_color[index]   = color;
    m_is_color_dirty = true;
}
