#include "draw_texture.h"

#include <cassert>

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

std::vector<uint32_t> const SQUARE_INDICES = {0, 1, 2, 0, 2, 3};

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
    auto const& renderer     = core->get<RendererService>();

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

    auto command_buffer = renderer->acquire_command_buffer();
    auto copy_pass      = command_buffer->start_copy_pass();

    m_vertex_buffer = renderer->create_vertex_buffer(std::span(square_vertices));
    copy_pass->upload(*m_vertex_buffer, std::span(square_vertices));

    m_index_buffer = renderer->create_index_buffer(std::span(SQUARE_INDICES));
    copy_pass->upload(*m_index_buffer, std::span(SQUARE_INDICES));

    m_color = float4(1.0F);
}

DrawTexture::~DrawTexture() = default;

void DrawTexture::update(GPUCopyPass const& /*copy_pass*/, uint64_t /*delta_time*/) {}

void DrawTexture::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_vertex_buffer);

    render_pass.push_vertex_uniform_data(0, &m_view_proj_matrix, sizeof(m_view_proj_matrix));
    render_pass.push_fragment_uniform_data(0, &m_color, sizeof(m_color));
    // render_pass.bind(source); // Bind texture that has scene rendered on it
    render_pass.draw(*m_index_buffer);
}
