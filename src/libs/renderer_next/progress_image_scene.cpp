#include "progress_image_scene.h"

#include <cassert>

#include <libs/core/core.h>
#include <shaders/ui/image_2d.h>

#include "impl_sdl/gpu_command_buffer.h"
#include "impl_sdl/gpu_index_buffer.h"
#include "impl_sdl/gpu_render_pass.h"
#include "impl_sdl/gpu_texture.h"
#include "impl_sdl/gpu_vertex_buffer.h"
#include "impl_sdl/renderer_sdl.h"

using namespace storm;
using namespace hlslpp;

using Vertex = shaders::ui::image_2d::VertexInput;

namespace
{

std::vector const SQUARE_VERTICES = {
    Vertex {{0.0F, 0.0F}, {0.0F, 0.0F}},
    Vertex {{1.0F, 0.0F}, {1.0F, 0.0F}},
    Vertex {{1.0F, 1.0F}, {1.0F, 1.0F}},
    Vertex {{0.0F, 1.0F}, {0.0F, 1.0F}},
};

std::vector<uint32_t> const SQUARE_INDICES = {0, 1, 2, 0, 2, 3};

constexpr char const PROGRESS_TEX[] = "loading/progress.tga";
constexpr char const BORDER_TEX[]   = "loading/int_border.tga";

// Loading screen textures are made for 4:3 screens
// TODO: maybe need to set up this in configuration files
constexpr auto ASPECT_RATIO = 4.0F / 3.0F;

std::pair<float, float> get_loading_picture_offset(FRect const& viewport, float const aspect_ratio)
{
    float dy = 0.0F;
    float dx = (viewport.width() - (aspect_ratio * viewport.height())) / 2.0F;
    if (dx < 10.0F) {
        dx = 0.0F;
    } else {
        dy = 25.0F;
        dx = (viewport.width() - (aspect_ratio * (viewport.height() - (2.0F * dy)))) / 2.0F;
    }

    return std::make_pair(dx, dy);
}

}  // namespace

ProgressImageScene::ProgressImageScene(
    std::shared_ptr<IConfigLoader> const& config_loader, std::shared_ptr<RendererService> const& renderer)
{
    m_progress_info = main_config::progress_image_info(*config_loader);

    {
        auto command_buffer = renderer->acquire_command_buffer();
        auto copy_pass      = command_buffer->start_copy_pass();

        m_progress = renderer->create_texture(PROGRESS_TEX);
        if (m_progress_info.frame) {
            m_frame = renderer->create_texture(BORDER_TEX);
        }

        m_vertex_buffer_back     = renderer->create_vertex_buffer(SQUARE_VERTICES);
        m_vertex_buffer_progress = renderer->create_vertex_buffer(SQUARE_VERTICES);
        m_index_buffer           = renderer->create_index_buffer(SQUARE_INDICES);

        renderer->upload_pending_data(*copy_pass);
    }

    m_pipeline = renderer->create_pipeline<Vertex>("ui/image_2d", "ui/picture");

    auto const viewport = renderer->get_viewport();
    auto const proj_mat =
        float4x4::orthographic(projection(frustum(viewport.left, viewport.right, viewport.bottom, viewport.top, -1.0F, 1.0F), zclip::zero));

    // Set orthographic projection matrix for all UBOs
    m_progress_ubo.view_proj = proj_mat;
    m_picture_ubo.view_proj  = proj_mat;

    // Background always scaled to entire screen
    m_background_ubo.model     = float4x4::scale(viewport.right - viewport.left, viewport.bottom - viewport.top, 1.0F);
    m_background_ubo.view_proj = proj_mat;

    m_fragment_ubo.diffuse = float4(1.0F);
}

void ProgressImageScene::update(GPUCopyPass const& copy_pass, uint64_t const /*delta_time*/)
{
    // Animated texture
    process_progress(copy_pass);

    // Recalculate texture positions
    update_picture_matrices();
    update_progress_matrices();
}

void ProgressImageScene::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.push_fragment_uniform_data(0, m_fragment_ubo);

    if (m_background) {
        render_pass.push_vertex_uniform_data(0, m_background_ubo);
        render_pass.bind(*m_vertex_buffer_back);
        render_pass.bind(*m_background);
        render_pass.draw(*m_index_buffer);
    }

    if (m_picture) {
        render_pass.push_vertex_uniform_data(0, m_picture_ubo);
        render_pass.bind(*m_vertex_buffer_back);
        render_pass.bind(*m_picture);
        render_pass.draw(*m_index_buffer);

        if (m_frame) {
            render_pass.bind(*m_frame);
            render_pass.draw(*m_index_buffer);
        }
    }

    render_pass.push_vertex_uniform_data(0, m_progress_ubo);
    render_pass.bind(*m_vertex_buffer_progress);
    render_pass.bind(*m_progress);
    render_pass.draw(*m_index_buffer);
}

void ProgressImageScene::set_picture(std::shared_ptr<GPUTexture> const& image)
{
    assert(image);
    m_picture = image;
}

void ProgressImageScene::set_background(std::shared_ptr<GPUTexture> const& image)
{
    assert(image);
    m_background = image;
}

void ProgressImageScene::process_progress(GPUCopyPass const& copy_pass)
{
    // Position of the current frame
    int32_t const fx = m_current_frame % m_progress_info.h_frames_count;
    int32_t const fy = m_current_frame / m_progress_info.h_frames_count;

    // Frame grid size
    auto const x_count = static_cast<float>(m_progress_info.h_frames_count);
    auto const y_count = static_cast<float>(m_progress_info.v_frames_count);

    std::vector const progress_tex_buffer = {
        // left-top
        decltype(Vertex::tex_coord)(fx / x_count, fy / y_count),
        // right-top
        decltype(Vertex::tex_coord)((fx + 1) / x_count, fy / y_count),
        // right-bottom
        decltype(Vertex::tex_coord)((fx + 1) / x_count, (fy + 1) / y_count),
        // left-bottom
        decltype(Vertex::tex_coord)(fx / x_count, (fy + 1) / y_count),
    };

    auto const progress_update_info =
        std::vector(4, BufferUpdateInfo {.offset = offsetof(Vertex, tex_coord), .size = sizeof(Vertex::tex_coord)});
    copy_pass.update_buffer(*m_vertex_buffer_progress, progress_update_info, progress_tex_buffer, sizeof(Vertex));

    ++m_current_frame;
    if (m_current_frame >= x_count * y_count) {
        m_current_frame = 0;
    }
}

void ProgressImageScene::update_picture_matrices()
{
    auto const& renderer = core->get<RendererService>();
    auto const  viewport = renderer->get_viewport();

    auto const [offset_x, offset_y] = get_loading_picture_offset(viewport, ASPECT_RATIO);

    auto const picture_width  = viewport.width() - (offset_x * 2);
    auto const picture_height = viewport.height() - (offset_y * 2);

    // Calculate background position
    auto const translation_mat = float4x4::translation(offset_x, offset_y, 0.0F);
    auto const scale_mat       = float4x4::scale(picture_width, picture_height, 1.0F);

    m_picture_ubo.model = mul(scale_mat, translation_mat);
}

void ProgressImageScene::update_progress_matrices()
{
    auto const& renderer = core->get<RendererService>();
    auto const  viewport = renderer->get_viewport();

    // Loading screen textures are made for 4:3 screens
    auto const [offset_x, offset_y] = get_loading_picture_offset(viewport, ASPECT_RATIO);

    auto const picture_width  = viewport.width() - (offset_x * 2);
    auto const picture_height = viewport.height() - (offset_y * 2);

    // Calculate progress animation position
    auto const translation_mat = float4x4::translation(
        (picture_width * m_progress_info.relative_x) + offset_x, (picture_height * m_progress_info.relative_y) + offset_y, 0.0F);

    auto const scale_mat = float4x4::scale(
        picture_width * m_progress_info.relative_width, picture_height * m_progress_info.relative_height * ASPECT_RATIO, 1.0F);

    m_progress_ubo.model = mul(scale_mat, translation_mat);
}
