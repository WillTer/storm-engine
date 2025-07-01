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

using namespace storm;
using namespace hlslpp;

namespace
{

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

constexpr auto VERTEX_SHADER_INFO = ShaderInfo {
    .num_samplers         = 0,
    .num_storage_textures = 0,
    .num_storage_buffers  = 0,
    .num_uniform_buffers  = 1,
};

constexpr auto FRAGMENT_SHADER_INFO = ShaderInfo {
    .num_samplers         = 1,
    .num_storage_textures = 0,
    .num_storage_buffers  = 0,
    .num_uniform_buffers  = 1,
};

constexpr char VERTEX_SHADER[]   = "picture_vs";
constexpr char FRAGMENT_SHADER[] = "picture_fs";

}  // namespace

Picture::Picture(
    GPUCopyPass const& copy_pass, std::filesystem::path const& texture, storm::FRect const& texture_rect /*= default_texture_rect()*/)
{
    auto const& asset_server = core->get<AssetServer>();
    auto const& renderer     = core->get<RendererService>();

    auto const texture_asset = asset_server->load_texture_file(texture);
    m_texture                = renderer->create_texture(texture_asset.header);
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

    render_pass.push_vertex_uniform_data(0, m_ubo);
    render_pass.push_fragment_uniform_data(0, m_color);
    render_pass.bind(*m_texture);
    render_pass.draw(*m_index_buffer);
}

void Picture::initialize(GPUCopyPass const& copy_pass, storm::FRect const& texture_rect)
{
    auto const& asset_server = core->get<AssetServer>();
    auto const& renderer     = core->get<RendererService>();

    auto const vertex_shader_asset   = asset_server->load_shader_file(VERTEX_SHADER);
    auto const fragment_shader_asset = asset_server->load_shader_file(FRAGMENT_SHADER);

    m_pipeline = renderer->create_pipeline<VertexUI>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);

    auto const viewport = renderer->get_viewport();
    set_screen_rect(viewport);  // Use viewport rect for projection matrix by default

    auto const square_vertices = std::vector<VertexUI> {
        VertexUI {{0.0F, 0.0F, texture_rect.left, texture_rect.top}},
        VertexUI {{1.0F, 0.0F, texture_rect.right, texture_rect.top}},
        VertexUI {{1.0F, 1.0F, texture_rect.right, texture_rect.bottom}},
        VertexUI {{0.0F, 1.0F, texture_rect.left, texture_rect.bottom}},
    };

    m_vertex_buffer = renderer->create_vertex_buffer(std::span(square_vertices));
    copy_pass.upload(*m_vertex_buffer, std::span(square_vertices));

    m_index_buffer = renderer->create_index_buffer(std::span(SQUARE_INDICES));
    copy_pass.upload(*m_index_buffer, std::span(SQUARE_INDICES));

    m_color = float4(1.0F);

    auto const [width, height] = m_texture->get_dimensions();

    m_width  = static_cast<uint32_t>(width * std::fabs(texture_rect.right - texture_rect.left));
    m_height = static_cast<uint32_t>(height * std::fabs(texture_rect.bottom - texture_rect.top));
}

void Picture::recalculate_model_matrix()
{
    m_ubo.model_mat = mul(mul(m_scaling_mat, m_rotation_mat_z), m_translation_mat);
}

void Picture::set_screen_rect(storm::FRect const& rect)
{
    m_ubo.view_proj_mat =
        float4x4::orthographic(projection(frustum(rect.left, rect.right, rect.bottom, rect.top, -1.0F, 1.0F), zclip::zero));
}

void Picture::set_rect(storm::FRect const& rect)
{
    m_translation_mat = float4x4::translation(rect.left, rect.top, 0.0F);
    m_scaling_mat     = float4x4::scale(rect.width(), rect.height(), 1.0F);
    recalculate_model_matrix();
}

void Picture::set_rotation(float angle)
{
    m_rotation_mat_z = float4x4::rotation_z(angle);
    recalculate_model_matrix();
}

void Picture::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_color                 = float4(r, g, b, a);
}

auto Picture::get_dimensions() const -> std::pair<uint32_t, uint32_t>
{
    return std::make_pair(m_width, m_height);
}
