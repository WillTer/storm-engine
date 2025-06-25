#include "progress_image_view.h"

#include <cassert>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>

#include "i_buffer.h"
#include "i_pipeline.h"
#include "i_renderer_next.h"
#include "i_texture.h"

using namespace storm;

namespace
{

std::vector const SQUARE_VERTICES = {
    VertexBase {{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F}},
    VertexBase {{1.0F, 0.0F, 0.0F}, {1.0F, 0.0F}},
    VertexBase {{1.0F, 1.0F, 0.0F}, {1.0F, 1.0F}},
    VertexBase {{0.0F, 1.0F, 0.0F}, {0.0F, 1.0F}},
};

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
    .num_uniform_buffers  = 0,
};

}  // namespace

ProgressImageView::ProgressImageView()
{
    auto const& asset_server     = core->get<AssetServer>();
    auto const  progress_texture = asset_server->load_texture_file("loading/progress.tga.tx");  // FIXME: hardcode

    auto const& renderer = core->get<RendererNext>();
    m_progress           = renderer->load_texture(progress_texture);

    m_vertex_buffer = renderer->load_vertex_buffer(SQUARE_VERTICES);
    m_index_buffer  = renderer->load_index_buffer(SQUARE_INDICES);

    auto const vertex_shader_asset   = asset_server->load_shader_file("interface_vs");
    auto const fragment_shader_asset = asset_server->load_shader_file("interface_fs");

    m_pipeline =
        renderer->create_pipeline<VertexBase>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);

    auto const viewport               = renderer->get_viewport();
    auto const proj_mat               = glm::ortho(viewport.left, viewport.right, viewport.bottom, viewport.top);
    m_progress_ubo.m_model_matrix     = glm::scale(glm::mat4(1.0F), glm::vec3(20.F, 20.F, 1.0F));
    m_progress_ubo.m_view_proj_matrix = proj_mat;

    m_background_ubo.m_model_matrix =
        glm::scale(glm::mat4(1.0F), glm::vec3(viewport.right - viewport.left, viewport.bottom - viewport.top, 1.0F));
    m_background_ubo.m_view_proj_matrix = proj_mat;
}

void ProgressImageView::set_background(std::shared_ptr<ITexture> const& image)
{
    assert(image);
    m_background = image;
}

void ProgressImageView::present(uint64_t /*delta_time*/) const
{
    auto const& renderer = core->get<RendererNext>();
    renderer->start_pass();

    m_pipeline->bind_to_render_pass();
    m_vertex_buffer->bind_to_render_pass();
    m_index_buffer->bind_to_render_pass();

    renderer->push_vertex_unform_data(0, &m_background_ubo, sizeof(m_background_ubo));
    m_background->bind_to_render_pass();
    m_index_buffer->draw_indexed();

    renderer->push_vertex_unform_data(0, &m_progress_ubo, sizeof(m_progress_ubo));
    m_progress->bind_to_render_pass();
    m_index_buffer->draw_indexed();

    renderer->end_pass();
}
