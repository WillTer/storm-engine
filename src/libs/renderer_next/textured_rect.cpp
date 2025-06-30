#include "textured_rect.h"

#include <cassert>
#include <span>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>

#include "impl_sdl/gpu_command_buffer.h"
#include "impl_sdl/gpu_index_buffer.h"
#include "impl_sdl/gpu_texture.h"
#include "impl_sdl/gpu_vertex_buffer.h"
#include "impl_sdl/renderer_sdl.h"

using namespace storm;
using namespace hlslpp;

namespace
{

auto const SQUARE_VERTICES = std::vector<VertexUI> {
    VertexUI {{0.0F, 0.0F, 0.0F, 0.0F}},
    VertexUI {{1.0F, 0.0F, 1.0F, 0.0F}},
    VertexUI {{1.0F, 1.0F, 1.0F, 1.0F}},
    VertexUI {{0.0F, 1.0F, 0.0F, 1.0F}},
};

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

constexpr char VERTEX_SHADER[]   = "textured_rect_vs";
constexpr char FRAGMENT_SHADER[] = "textured_rect_fs";

}  // namespace

TexturedRect::TexturedRect(GPUCopyPass const& copy_pass, std::filesystem::path const& texture)
{
    auto const& asset_server = core->get<AssetServer>();
    auto const& renderer     = core->get<RendererService>();

    auto const vertex_shader_asset   = asset_server->load_shader_file(VERTEX_SHADER);
    auto const fragment_shader_asset = asset_server->load_shader_file(FRAGMENT_SHADER);

    m_pipeline = renderer->create_pipeline<VertexUI>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);

    auto const viewport = renderer->get_viewport();
    set_screen_rect(viewport);  // Use viewport rect for projection matrix by default

    m_vertex_buffer = renderer->create_vertex_buffer(std::span(SQUARE_VERTICES));
    copy_pass.upload(*m_vertex_buffer, std::span(SQUARE_VERTICES));

    m_index_buffer = renderer->create_index_buffer(std::span(SQUARE_INDICES));
    copy_pass.upload(*m_index_buffer, std::span(SQUARE_INDICES));

    auto const texture_asset = asset_server->load_texture_file(texture);
    m_texture                = renderer->create_texture(texture_asset.header);
    copy_pass.upload(*m_texture, std::span(texture_asset.data));

    m_color = float4(1.0F);
}

TexturedRect::~TexturedRect() = default;

void TexturedRect::update(GPUCopyPass const& /*copy_pass*/, uint64_t /*delta_time*/) {}

void TexturedRect::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, &m_ubo, sizeof(m_ubo));
    render_pass.push_fragment_uniform_data(0, &m_color, sizeof(m_color));
    render_pass.bind(*m_texture);
    render_pass.draw(*m_index_buffer);
}

void TexturedRect::set_screen_rect(storm::FRect const& rect)
{
    m_ubo.view_proj_mat =
        float4x4::orthographic(projection(frustum(rect.left, rect.right, rect.bottom, rect.top, -1.0F, 1.0F), zclip::zero));
}

void TexturedRect::set_position(storm::FPoint const& pos)
{
    m_translation   = float4x4::translation(pos.x, pos.y, 0.0F);
    m_ubo.model_mat = mul(m_scale, m_translation);
}

void TexturedRect::set_size(float width, float height)
{
    m_scale         = float4x4::scale(width, height, 1.0F);
    m_ubo.model_mat = mul(m_scale, m_translation);
}

void TexturedRect::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_color                 = float4(r, g, b, a);
}
