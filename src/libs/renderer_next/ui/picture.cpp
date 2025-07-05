#include "picture.h"

#include <cassert>
#include <span>

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

Picture::Picture(
    GPUCopyPass const& copy_pass, std::filesystem::path const& texture, storm::FRect const& texture_rect /*= default_texture_rect()*/)
{
    auto const& asset_server = core->get<AssetServer>();
    auto const& renderer     = core->get<RendererService>();

    auto const texture_asset = asset_server->load_texture_file(texture);
    m_texture                = renderer->create_texture(texture_asset.path_hashed, texture_asset.header);
    copy_pass.upload(*m_texture, std::span(texture_asset.data));

    initialize(copy_pass, texture_rect);
}

Picture::Picture(
    GPUCopyPass const&                 copy_pass,
    std::shared_ptr<GPUTexture> const& external_texture,
    storm::FRect const&                texture_rect /*= default_texture_rect()*/)
    : m_texture(external_texture)
{
    initialize(copy_pass, texture_rect);
}

Picture::~Picture() = default;

void Picture::update(GPUCopyPass const& /*copy_pass*/, uint64_t /*delta_time*/) {}

void Picture::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);
    render_pass.push_fragment_uniform_data(0, m_fragment_ubo);
    render_pass.bind(*m_texture);
    render_pass.draw(*m_index_buffer);
}

void Picture::initialize(GPUCopyPass const& copy_pass, storm::FRect const& texture_rect)
{
    auto const& renderer = core->get<RendererService>();

    m_pipeline = renderer->create_pipeline(IMAGE_2D_PIPELINE);

    auto const viewport = renderer->get_viewport();
    set_screen_rect(viewport);  // Use viewport rect for projection matrix by default

    auto const square_vertices = std::vector<ImageVertex> {
        ImageVertex {{0.0F, 0.0F, texture_rect.left, texture_rect.top}},
        ImageVertex {{1.0F, 0.0F, texture_rect.right, texture_rect.top}},
        ImageVertex {{1.0F, 1.0F, texture_rect.right, texture_rect.bottom}},
        ImageVertex {{0.0F, 1.0F, texture_rect.left, texture_rect.bottom}},
    };

    m_vertex_buffer = renderer->create_vertex_buffer(std::span(square_vertices));
    copy_pass.upload(*m_vertex_buffer, std::span(square_vertices));

    m_index_buffer = renderer->create_index_buffer(std::span(SQUARE_INDICES));
    copy_pass.upload(*m_index_buffer, std::span(SQUARE_INDICES));

    m_fragment_ubo.color = float4(1.0F);

    auto const [width, height] = m_texture->get_dimensions();

    m_rect = {
        0.0F, 0.0F, width * std::fabs(texture_rect.right - texture_rect.left), height * std::fabs(texture_rect.bottom - texture_rect.top)};
    m_width  = static_cast<uint32_t>(m_rect.width());
    m_height = static_cast<uint32_t>(m_rect.height());
}

void Picture::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_fragment_ubo.color    = float4(r, g, b, a);
}
