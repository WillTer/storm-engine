#include "rectangle.h"

#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_copy_pass.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_render_pass.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <shaders/ui/rectangle.h>

using namespace storm;
using namespace storm::renderer::ui;
using namespace hlslpp;

using Vertex = shaders::ui::rectangle::VertexInput;

namespace
{

auto const SQUARE_INDICES      = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};
auto const SQUARE_LINE_INDICES = std::vector<uint32_t> {0, 1, 1, 2, 2, 3, 3, 0};
auto const SQUARE_VERTICES     = std::vector<Vertex> {
    Vertex {{0.0F, 0.0F}, {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex {{1.0F, 0.0F}, {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex {{1.0F, 1.0F}, {1.0F, 1.0F, 1.0F, 1.0F}},
    Vertex {{0.0F, 1.0F}, {1.0F, 1.0F, 1.0F, 1.0F}},
};

}  // namespace

Rectangle::Rectangle(storm::Color const& color, std::optional<std::string> const& technique /*= std::nullopt*/)
{
    m_colors.resize(4);
    std::fill(m_colors.begin(), m_colors.end(), color.to_float4());

    initialize(technique);
}

Rectangle::Rectangle(uint32_t const& color, std::optional<std::string> const& technique /*= std::nullopt*/)
    : Rectangle(Color::from_hex(color), technique)
{
}

Rectangle::Rectangle(std::array<storm::Color, 4> const& colors, std::optional<std::string> const& technique /*= std::nullopt*/)
{
    m_colors.clear();
    std::transform(colors.begin(), colors.end(), std::back_inserter(m_colors), [](storm::Color const& c) { return c.to_float4(); });

    initialize(technique);
}

Rectangle::Rectangle(std::array<uint32_t, 4> const& colors, std::optional<std::string> const& technique /*= std::nullopt*/)
{
    m_colors.clear();
    std::transform(
        colors.begin(), colors.end(), std::back_inserter(m_colors), [](uint32_t const& c) { return Color::from_hex(c).to_float4(); });

    initialize(technique);
}

Rectangle::~Rectangle() = default;

void Rectangle::set_vertices_colors(std::array<storm::Color, 4> const& colors)
{
    m_colors.clear();
    std::transform(colors.begin(), colors.end(), std::back_inserter(m_colors), [](storm::Color const& c) { return c.to_float4(); });

    m_need_update = true;
}

void Rectangle::set_vertices_colors(std::array<uint32_t, 4> const& colors)
{
    m_colors.clear();
    std::transform(
        colors.begin(), colors.end(), std::back_inserter(m_colors), [](uint32_t const& c) { return Color::from_hex(c).to_float4(); });

    m_need_update = true;
}

void Rectangle::set_vertices_color(storm::Color const& color)
{
    m_colors.resize(4);
    std::fill(m_colors.begin(), m_colors.end(), color.to_float4());

    m_need_update = true;
}

void Rectangle::set_vertices_color(uint32_t const& color)
{
    set_vertices_color(Color::from_hex(color));
}

void Rectangle::create_default_pipeline(std::string const& fragment_shader /*= {}*/)
{
    auto const& renderer = core->get<RendererService>();
    m_pipeline           = renderer->create_pipeline<Vertex>("ui/rectangle", fragment_shader.empty() ? "ui/rectangle" : fragment_shader);
}

void Rectangle::set_technique(std::string const& technique, std::string const& vertex_shader /*= {}*/)
{
    auto const& renderer = core->get<RendererService>();
    auto const  info     = renderer->get_technique_info(technique);
    m_pipeline           = renderer->create_pipeline<Vertex>(vertex_shader.empty() ? "ui/rectangle" : vertex_shader, info);

    // Recreate index buffer using indices compatible with technique
    m_index_buffer =
        renderer->create_index_buffer(info.pipeline.fill_mode == technique::FillMode::Line ? SQUARE_LINE_INDICES : SQUARE_INDICES);
}

void Rectangle::update(GPUCopyPass const& copy_pass, uint64_t /*delta_time*/)
{
    if (m_need_update) {
        auto const update_info = std::vector(4, BufferUpdateInfo {.offset = offsetof(Vertex, diffuse), .size = sizeof(Vertex::diffuse)});
        copy_pass.update_buffer(*m_vertex_buffer, update_info, m_colors, sizeof(Vertex));

        m_need_update = false;
    }
}

void Rectangle::draw(GPURenderPass const& render_pass) const
{
    if (!m_pipeline) {
        return;
    }

    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);

    render_pass.draw(*m_index_buffer);
}

void Rectangle::initialize(std::optional<std::string> const& technique)
{
    std::vector vertices = SQUARE_VERTICES;
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i].diffuse = m_colors[i];
    }

    auto const& renderer = core->get<RendererService>();
    m_vertex_buffer      = renderer->create_vertex_buffer(vertices);

    if (technique.has_value()) {
        set_technique(technique.value());
    } else {
        m_index_buffer = renderer->create_index_buffer(SQUARE_INDICES);
        create_default_pipeline();
    }
}
