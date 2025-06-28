#include "draw_texture.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>

#include "i_buffer.h"
#include "i_pipeline.h"
#include "i_renderer_next.h"
#include "i_texture.h"

using namespace storm;
using namespace hlslpp;

namespace
{

std::vector<uint16_t> const SQUARE_INDICES = {0, 1, 2, 0, 2, 3};

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

constexpr char VERTEX_SHADER[]   = "draw_texture_vs";
constexpr char FRAGMENT_SHADER[] = "draw_texture_fs";

}  // namespace

DrawTexture::DrawTexture()
{
    auto const& asset_server = core->get<AssetServer>();
    auto const& renderer     = core->get<RendererNext>();

    auto const vertex_shader_asset   = asset_server->load_shader_file(VERTEX_SHADER);
    auto const fragment_shader_asset = asset_server->load_shader_file(FRAGMENT_SHADER);

    m_pipeline = renderer->create_pipeline<VertexUI>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);

    auto const viewport = renderer->get_viewport();
    m_view_proj_matrix =
        float4x4::orthographic(projection(frustum(viewport.left, viewport.right, viewport.bottom, viewport.top, -1.0F, 1.0F), zclip::zero));

    std::vector const square_vertices = {
        VertexUI {{0.0F, 0.0F, 0.0F, 0.0F}},
        VertexUI {{viewport.width(), 0.0F, 1.0F, 0.0F}},
        VertexUI {{viewport.width(), viewport.height(), 1.0F, 1.0F}},
        VertexUI {{0.0F, viewport.height(), 0.0F, 1.0F}},
    };

    m_vertex_buffer = renderer->load_vertex_buffer(square_vertices);
    m_index_buffer  = renderer->load_index_buffer(SQUARE_INDICES);
    m_color         = float4(1.0F);
}

DrawTexture::~DrawTexture() = default;

void DrawTexture::update(uint64_t const /*delta_time*/) {}

void DrawTexture::present(ITexture& source) const
{
    auto const& renderer = core->get<RendererNext>();

    renderer->start_render_pass();

    m_pipeline->bind_to_render_pass();
    m_index_buffer->bind_to_render_pass();
    m_vertex_buffer->bind_to_render_pass();

    renderer->push_vertex_uniform_data(0, &m_view_proj_matrix, sizeof(m_view_proj_matrix));
    renderer->push_fragment_uniform_data(0, &m_color, sizeof(m_color));
    source.bind_to_render_pass();  // Bind texture that has scene rendered on it
    m_index_buffer->draw_indexed();

    renderer->end_render_pass();
}
