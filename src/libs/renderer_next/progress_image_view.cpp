#include "progress_image_view.h"

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

std::vector const SQUARE_VERTICES = {
    VertexWithDiffuse {VertexBase {{0.0F, 0.0F, 0.0F}, {0.0F, 0.0F}}, {1.0F, 1.0F, 1.0F, 1.0F}},
    VertexWithDiffuse {VertexBase {{1.0F, 0.0F, 0.0F}, {1.0F, 0.0F}}, {1.0F, 1.0F, 1.0F, 1.0F}},
    VertexWithDiffuse {VertexBase {{1.0F, 1.0F, 0.0F}, {1.0F, 1.0F}}, {1.0F, 1.0F, 1.0F, 1.0F}},
    VertexWithDiffuse {VertexBase {{0.0F, 1.0F, 0.0F}, {0.0F, 1.0F}}, {1.0F, 1.0F, 1.0F, 1.0F}},
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

constexpr char VERTEX_SHADER[]   = "interface_vs";
constexpr char FRAGMENT_SHADER[] = "interface_fs";

// Loading screen textures are made for 4:3 screens
// TODO: maybe need to set up this in configuration files
constexpr auto ASPECT_RATIO = 4.0F / 3.0F;

std::pair<float, float> get_loading_picture_offset(FRect const& viewport, float const aspect_ratio)
{
    float dy = 0.0F;
    float dx = (viewport.width() - aspect_ratio * viewport.height()) / 2.0F;
    if (dx < 10.0F) {
        dx = 0.0F;
    } else {
        dy = 25.0F;
        dx = (viewport.width() - aspect_ratio * (viewport.height() - 2.0F * dy)) / 2.0F;
    }

    return std::make_pair(dx, dy);
}

}  // namespace

ProgressImageView::ProgressImageView(
    std::shared_ptr<AssetServer> const&   asset_server,
    std::shared_ptr<IConfigLoader> const& config_loader,
    std::shared_ptr<RendererNext> const&  renderer)
{
    m_progress_info = main_config::progress_image_info(*config_loader);

    auto const progress_texture = asset_server->load_texture_file("loading/progress.tga");  // FIXME: hardcode
    m_progress                  = renderer->load_texture(progress_texture);

    if (m_progress_info.frame) {
        auto const frame_texture = asset_server->load_texture_file("interfaces/int_border.tga");  // FIXME: hardcode
        m_frame                  = renderer->load_texture(frame_texture);
    }

    m_vertex_buffer_back     = renderer->load_vertex_buffer(SQUARE_VERTICES);
    m_vertex_buffer_progress = renderer->load_vertex_buffer(SQUARE_VERTICES);
    m_index_buffer           = renderer->load_index_buffer(SQUARE_INDICES);

    auto const vertex_shader_asset   = asset_server->load_shader_file(VERTEX_SHADER);
    auto const fragment_shader_asset = asset_server->load_shader_file(FRAGMENT_SHADER);

    m_pipeline =
        renderer->create_pipeline<VertexWithDiffuse>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);

    auto const viewport = renderer->get_viewport();
    auto const proj_mat =
        float4x4::orthographic(projection(frustum(viewport.left, viewport.right, viewport.bottom, viewport.top, -1.0F, 1.0F), zclip::zero));

    // Set orthographic projection matrix for all UBOs
    m_progress_ubo.m_view_proj_matrix = proj_mat;
    m_picture_ubo.m_view_proj_matrix  = proj_mat;

    // Background always scaled to entire screen
    m_background_ubo.m_model_matrix     = float4x4::scale(viewport.right - viewport.left, viewport.bottom - viewport.top, 1.0F);
    m_background_ubo.m_view_proj_matrix = proj_mat;
}

void ProgressImageView::set_picture(std::shared_ptr<ITexture> const& image)
{
    assert(image);
    m_picture = image;
}

void ProgressImageView::set_background(std::shared_ptr<ITexture> const& image)
{
    assert(image);
    m_background = image;
}

void ProgressImageView::update(uint64_t const delta_time)
{
    // Fader
    process_fader(delta_time);

    // Animated texture
    process_progress();

    // Recalculate texture positions
    update_picture_matrices();
    update_progress_matrices();
}

void ProgressImageView::present() const
{
    auto const& renderer = core->get<RendererNext>();
    renderer->start_pass();

    m_pipeline->bind_to_render_pass();
    m_index_buffer->bind_to_render_pass();

    if (m_background) {
        renderer->push_vertex_unform_data(0, &m_background_ubo, sizeof(m_background_ubo));
        m_vertex_buffer_back->bind_to_render_pass();
        m_background->bind_to_render_pass();
        m_index_buffer->draw_indexed();
    }

    if (m_picture) {
        renderer->push_vertex_unform_data(0, &m_picture_ubo, sizeof(m_picture_ubo));
        m_vertex_buffer_back->bind_to_render_pass();
        m_picture->bind_to_render_pass();
        m_index_buffer->draw_indexed();
        if (m_frame) {
            m_frame->bind_to_render_pass();
            m_index_buffer->draw_indexed();
        }
    }

    renderer->push_vertex_unform_data(0, &m_progress_ubo, sizeof(m_progress_ubo));
    m_vertex_buffer_progress->bind_to_render_pass();
    m_progress->bind_to_render_pass();
    m_index_buffer->draw_indexed();

    renderer->end_pass();
}

void ProgressImageView::set_fade_speed(float const speed)
{
    m_fade_speed = speed;
}

void ProgressImageView::set_fade_alpha(float const alpha)
{
    m_fade_alpha = std::clamp(alpha, 0.0F, 1.0F);
}

void ProgressImageView::process_fader(uint64_t const delta_time)
{
    auto const alpha_buffer = std::array {m_fade_alpha, m_fade_alpha, m_fade_alpha, m_fade_alpha};
    auto const info =
        std::vector(4, BufferUpdateInfo {.offset = offsetof(VertexWithDiffuse, diffuse) + sizeof(float) * 3, .size = sizeof(float)});

    m_vertex_buffer_back->update_data(info, alpha_buffer.data(), sizeof(VertexWithDiffuse));

    m_fade_alpha = std::clamp(m_fade_alpha + delta_time * 0.001F * m_fade_speed, 0.0F, 1.0F);
}

void ProgressImageView::process_progress()
{
    // Position of the current frame
    int32_t const fx = m_current_frame % m_progress_info.h_frames_count;
    int32_t const fy = m_current_frame / m_progress_info.v_frames_count;

    // Frame grid size
    auto const x_count = static_cast<float>(m_progress_info.h_frames_count);
    auto const y_count = static_cast<float>(m_progress_info.v_frames_count);

    std::array const progress_tex_buffer = {
        // left-top
        FPoint {fx / x_count, fy / y_count},
        // right-top
        FPoint {(fx + 1) / x_count, fy / y_count},
        // right-bottom
        FPoint {(fx + 1) / x_count, (fy + 1) / y_count},
        // left-bottom
        FPoint {fx / x_count, (fy + 1) / y_count},
    };

    auto const progress_update_info = std::vector(4, BufferUpdateInfo {.offset = offsetof(VertexBase, uv), .size = sizeof(VertexBase::uv)});
    m_vertex_buffer_progress->update_data(progress_update_info, progress_tex_buffer.data(), sizeof(VertexWithDiffuse));

    ++m_current_frame;
    if (m_current_frame >= x_count * y_count) m_current_frame = 0;
}

void ProgressImageView::update_picture_matrices()
{
    auto const& renderer = core->get<RendererNext>();
    auto const  viewport = renderer->get_viewport();

    auto const [offset_x, offset_y] = get_loading_picture_offset(viewport, ASPECT_RATIO);

    auto const picture_width  = viewport.width() - offset_x * 2;
    auto const picture_height = viewport.height() - offset_y * 2;

    // Calculate background position
    auto const translation_mat = float4x4::translation(offset_x, offset_y, 0.0F);
    auto const scale_mat       = float4x4::scale(picture_width, picture_height, 1.0F);

    m_picture_ubo.m_model_matrix = mul(scale_mat, translation_mat);
}

void ProgressImageView::update_progress_matrices()
{
    auto const& renderer = core->get<RendererNext>();
    auto const  viewport = renderer->get_viewport();

    // Loading screen textures are made for 4:3 screens
    auto const [offset_x, offset_y] = get_loading_picture_offset(viewport, ASPECT_RATIO);

    auto const picture_width  = viewport.width() - offset_x * 2;
    auto const picture_height = viewport.height() - offset_y * 2;

    // Calculate progress animation position
    auto const translation_mat = float4x4::translation(
        picture_width * m_progress_info.relative_x + offset_x, picture_height * m_progress_info.relative_y + offset_y, 0.0F);

    auto const scale_mat = float4x4::scale(
        picture_width * m_progress_info.relative_width, picture_height * m_progress_info.relative_height * ASPECT_RATIO, 1.0F);

    m_progress_ubo.m_model_matrix = mul(scale_mat, translation_mat);
}
