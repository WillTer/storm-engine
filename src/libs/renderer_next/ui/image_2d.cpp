#include "image_2d.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_copy_pass.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_render_pass.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <shaders/ui/image_2d.h>

using namespace storm;
using namespace storm::renderer::ui;
using namespace hlslpp;

using Vertex = shaders::ui::image_2d::VertexInput;

namespace
{

auto const SQUARE_INDICES  = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};
auto const SQUARE_VERTICES = std::vector<Vertex> {
    Vertex {{0.0F, 0.0F}, {0.0F, 0.0F}},
    Vertex {{1.0F, 0.0F}, {1.0F, 0.0F}},
    Vertex {{1.0F, 1.0F}, {1.0F, 1.0F}},
    Vertex {{0.0F, 1.0F}, {0.0F, 1.0F}},
};

}  // namespace

Image2D::Image2D(
    std::filesystem::path const& texture, storm::FRect const& uv /*= {}*/, std::optional<std::string> const& technique /*= std::nullopt*/)
{
    auto const& renderer = core->get<RendererService>();
    m_texture            = renderer->create_texture(texture.string());

    initialize(uv, technique);
}

Image2D::Image2D(
    std::shared_ptr<GPUTexture> const& external_texture,
    storm::FRect const&                uv /*= {}*/,
    std::optional<std::string> const&  technique /*= std::nullopt*/)
{
    m_texture = external_texture;
    initialize(uv, technique);
}

Image2D::~Image2D() = default;

auto Image2D::get_uv() const -> std::vector<float2> const&
{
    return m_texture_uv;
}

void Image2D::set_uv_rect(storm::FRect const& uv)
{
    m_texture_uv = {
        {uv.left, uv.top},
        {uv.right, uv.top},
        {uv.right, uv.bottom},
        {uv.left, uv.bottom},
    };

    update_rect();

    m_need_update = true;
}

void Image2D::set_uv(std::array<float2, 4> const& uv)
{
    m_texture_uv.clear();
    m_texture_uv.insert(m_texture_uv.end(), uv.begin(), uv.end());

    update_rect();

    m_need_update = true;
}

void Image2D::set_ubo_color(storm::Color const& color)
{
    m_fragment_ubo.diffuse = color.to_float4();
}

void Image2D::create_default_pipeline(std::string const& fragment_shader /*= {}*/)
{
    auto const& renderer = core->get<RendererService>();
    m_pipeline           = renderer->create_pipeline<Vertex>("ui/image_2d", fragment_shader.empty() ? "ui/picture" : fragment_shader);
}

void Image2D::set_technique(std::string const& technique, std::string const& vertex_shader /*= {}*/)
{
    auto const& renderer = core->get<RendererService>();
    auto const  info     = renderer->get_technique_info(technique);
    m_pipeline           = renderer->create_pipeline<Vertex>(vertex_shader.empty() ? "ui/image_2d" : vertex_shader, info);

    if (m_texture) {
        m_texture->set_sampler(renderer->create_texture_sampler(info));
    }
}

void Image2D::update(GPUCopyPass const& copy_pass, uint64_t /*delta_time*/)
{
    if (m_need_update) {
        auto const update_info =
            std::vector(4, BufferUpdateInfo {.offset = offsetof(Vertex, tex_coord), .size = sizeof(Vertex::tex_coord)});
        copy_pass.update_buffer(*m_vertex_buffer, update_info, m_texture_uv, sizeof(Vertex));

        m_need_update = false;
    }
}

void Image2D::draw(GPURenderPass const& render_pass) const
{
    if (!m_pipeline || !m_texture) {
        return;
    }

    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);
    render_pass.push_fragment_uniform_data(0, m_fragment_ubo);

    render_pass.bind(*m_texture);

    render_pass.draw(*m_index_buffer);
}

void Image2D::initialize(storm::FRect const& uv, std::optional<std::string> const& technique)
{
    std::vector vertices = SQUARE_VERTICES;
    if (!uv.is_empty()) {
        m_texture_uv = {
            {uv.left, uv.top},
            {uv.right, uv.top},
            {uv.right, uv.bottom},
            {uv.left, uv.bottom},
        };

        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].tex_coord = m_texture_uv[i];
        }
    } else {
        m_texture_uv = {
            {0.0F, 0.0F},
            {1.0F, 0.0F},
            {1.0F, 1.0F},
            {0.0F, 1.0F},
        };
    }

    auto const& renderer = core->get<RendererService>();
    m_vertex_buffer      = renderer->create_vertex_buffer(vertices);
    m_index_buffer       = renderer->create_index_buffer(SQUARE_INDICES);

    if (technique.has_value()) {
        set_technique(technique.value());
    } else {
        create_default_pipeline();
    }

    update_rect();
}

void Image2D::update_rect()
{
    if (!m_texture) {
        return;
    }

    auto const [width, height] = m_texture->get_dimensions();

    float min_x = m_texture_uv[0].x;
    float max_x = m_texture_uv[0].x;
    float min_y = m_texture_uv[0].y;
    float max_y = m_texture_uv[0].y;

    for (size_t i = 1; i < m_texture_uv.size(); ++i) {
        min_x = std::min<float>(m_texture_uv[i].x, min_x);
        max_x = std::max<float>(m_texture_uv[i].x, max_x);
        min_y = std::min<float>(m_texture_uv[i].y, min_y);
        max_y = std::max<float>(m_texture_uv[i].y, max_y);
    }

    set_rect({width * min_x, height * min_y, width * max_x, height * max_y});
}
