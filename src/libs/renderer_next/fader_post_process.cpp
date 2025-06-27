#include "fader_post_process.h"

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

FaderPostProcess::FaderPostProcess()
{
    auto const& asset_server = core->get<AssetServer>();
    auto const& renderer     = core->get<RendererNext>();

    auto const vertex_shader_asset   = asset_server->load_shader_file(VERTEX_SHADER);
    auto const fragment_shader_asset = asset_server->load_shader_file(FRAGMENT_SHADER);

    m_pipeline =
        renderer->create_pipeline<VertexWithDiffuse>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);

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

    m_render_target = renderer->create_texture_target();
    m_color         = float4(1.0F);
}

void FaderPostProcess::update(uint64_t const delta_time)
{
    constexpr float ms_to_s = 0.001F;  // Multiplier to convert milliseconds to seconds

    m_fade_alpha = std::clamp(m_fade_alpha + (delta_time * ms_to_s * m_fade_speed), 0.0F, 1.0F);
    m_color.a    = m_fade_alpha;

    m_is_fade_finished = (m_fade_speed < 0.0F && m_fade_alpha <= std::numeric_limits<float>::epsilon())  // Fade out
        || (m_fade_speed > 0.0F && (m_fade_alpha - 1.0F) <= std::numeric_limits<float>::epsilon());      // Fade in
}

void FaderPostProcess::render(ITexture& scene_target) const
{
    auto const& renderer = core->get<RendererNext>();

    m_render_target->start_render_pass(false);

    m_pipeline->bind_to_render_pass();
    m_index_buffer->bind_to_render_pass();
    m_vertex_buffer->bind_to_render_pass();

    renderer->push_vertex_uniform_data(0, &m_view_proj_matrix, sizeof(m_view_proj_matrix));
    renderer->push_fragment_uniform_data(0, &m_color, sizeof(m_color));
    scene_target.bind_to_render_pass();  // Bind texture that has scene rendered on it
    m_index_buffer->draw_indexed();

    m_render_target->end_render_pass();

    if (m_next) { m_next->render(*m_render_target); }
}

void FaderPostProcess::set_next(std::shared_ptr<IPostProcessor> const& next)
{
    m_next = next;
}

std::shared_ptr<IPostProcessor> FaderPostProcess::get_next() const
{
    return m_next;
}

void FaderPostProcess::start_fade(float alpha, float speed)
{
    m_fade_alpha       = std::clamp(alpha, 0.0F, 1.0F);
    m_fade_speed       = speed;
    m_is_fade_finished = false;
}

bool FaderPostProcess::is_fade_finished() const
{
    return m_is_fade_finished;
}
