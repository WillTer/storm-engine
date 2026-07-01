#include "image_2d.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <libs/renderer_next/pipeline_names.h>
#include <shaders/ui/image_2d.h>

using namespace storm;
using namespace hlslpp;

using ImageVertex = shaders::image_2d::VertexInput;

namespace
{

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

}  // namespace

Image2D::Image2D(std::filesystem::path const& texture, storm::FRect const& texture_rect /*= default_texture_rect()*/)
{
    auto const& renderer = core->get<RendererService>();
    m_texture            = renderer->create_texture(texture.string());

    initialize(texture_rect);
}

Image2D::Image2D(std::shared_ptr<GPUTexture> const& external_texture, storm::FRect const& texture_rect /*= default_texture_rect()*/)
    : m_texture(external_texture)
{
    initialize(texture_rect);
}

Image2D::~Image2D() = default;

void Image2D::update(GPUCopyPass const& /*copy_pass*/, uint64_t /*delta_time*/) {}

void Image2D::draw(GPURenderPass const& render_pass) const
{
    if (!m_texture) {
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

void Image2D::initialize(storm::FRect const& texture_rect)
{
    auto const& renderer = core->get<RendererService>();

    m_pipeline = renderer->create_pipeline(IMAGE_2D_PIPELINE);

    auto const vertex_data = std::vector<ImageVertex> {
        ImageVertex {{0.0F, 0.0F, texture_rect.left, texture_rect.top}},
        ImageVertex {{1.0F, 0.0F, texture_rect.right, texture_rect.top}},
        ImageVertex {{1.0F, 1.0F, texture_rect.right, texture_rect.bottom}},
        ImageVertex {{0.0F, 1.0F, texture_rect.left, texture_rect.bottom}},
    };

    m_vertex_buffer = renderer->create_vertex_buffer(vertex_data);
    m_index_buffer  = renderer->create_index_buffer(SQUARE_INDICES);

    m_fragment_ubo.color = float4(1.0F);

    if (m_texture) {
        auto const [width, height] = m_texture->get_dimensions();

        m_rect   = {0.0F, 0.0F, width * std::fabs(texture_rect.width()), height * std::fabs(texture_rect.height())};
        m_width  = static_cast<uint32_t>(m_rect.width());
        m_height = static_cast<uint32_t>(m_rect.height());
    }
}

void Image2D::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_fragment_ubo.color    = float4(r, g, b, a);
}
