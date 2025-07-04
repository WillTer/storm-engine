#include "texture_sequence.h"

#include <cassert>
#include <span>

#include <libs/asset_server/asset_server.h>
#include <libs/config/texture_sequence.h>
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

auto const SQUARE_VERTICES = std::vector<VertexUI> {
    VertexUI {{0.0F, 0.0F, 0.0F, 0.0F}},
    VertexUI {{1.0F, 0.0F, 1.0F, 0.0F}},
    VertexUI {{1.0F, 1.0F, 1.0F, 1.0F}},
    VertexUI {{0.0F, 1.0F, 0.0F, 1.0F}},
};

auto const SEQUENCE_SQUARE_VERTICES = std::vector<VertexUI> {
    VertexUI {{-1.0F, -1.0F, 0.0F, 1.0F}},
    VertexUI {{1.0F, -1.0F, 0.0F, 1.0F}},
    VertexUI {{1.0F, 1.0F, 0.0F, 1.0F}},
    VertexUI {{-1.0F, 1.0F, 0.0F, 1.0F}},
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

constexpr char VERTEX_SHADER[]   = "picture_vs";
constexpr char FRAGMENT_SHADER[] = "picture_fs";

constexpr char SEQUENCE_VERTEX_SHADER[]   = "texture_sequence_vs";
constexpr char SEQUENCE_FRAGMENT_SHADER[] = "texture_sequence_fs";

}  // namespace

TextureSequence::TextureSequence(GPUCopyPass const& copy_pass, std::string const& name)
{
    auto const& asset_server  = core->get<AssetServer>();
    auto const& renderer      = core->get<RendererService>();
    auto const& config_loader = core->get<IConfigLoader>();

    auto const info = storm::texture_sequence::info(*config_loader, name);

    auto const texture_asset = asset_server->load_texture_file(info.texture_file);
    m_texture                = renderer->create_texture(texture_asset.header);
    copy_pass.upload(*m_texture, std::span(texture_asset.data));

    m_target = renderer->create_texture_target(info.width, info.height);

    {
        auto const vertex_shader_asset   = asset_server->load_shader_file(VERTEX_SHADER);
        auto const fragment_shader_asset = asset_server->load_shader_file(FRAGMENT_SHADER);

        m_pipeline =
            renderer->create_pipeline<VertexUI>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);
    }

    {
        auto const vertex_shader_asset   = asset_server->load_shader_file(SEQUENCE_VERTEX_SHADER);
        auto const fragment_shader_asset = asset_server->load_shader_file(SEQUENCE_FRAGMENT_SHADER);

        m_sequence_pipeline =
            renderer->create_pipeline<VertexUI>(vertex_shader_asset, VERTEX_SHADER_INFO, fragment_shader_asset, FRAGMENT_SHADER_INFO);
    }

    auto const viewport = renderer->get_viewport();
    set_screen_rect(viewport);  // Use viewport rect for projection matrix by default

    m_sequence_vertex_buffer = renderer->create_vertex_buffer(std::span(SEQUENCE_SQUARE_VERTICES));
    copy_pass.upload(*m_sequence_vertex_buffer, std::span(SEQUENCE_SQUARE_VERTICES));

    m_vertex_buffer = renderer->create_vertex_buffer(std::span(SQUARE_VERTICES));
    copy_pass.upload(*m_vertex_buffer, std::span(SQUARE_VERTICES));

    m_index_buffer = renderer->create_index_buffer(std::span(SQUARE_INDICES));
    copy_pass.upload(*m_index_buffer, std::span(SQUARE_INDICES));

    m_color = float4(1.0F);

    m_time_delay = info.time_delay;
    m_width      = info.width;
    m_height     = info.height;

    m_sequence_ubo.frame          = 0;
    m_sequence_ubo.h_frames_count = info.h_frames_count;
    m_sequence_ubo.v_frames_count = info.v_frames_count;
    m_sequence_ubo.flip_h         = info.flip_h ? 1 : 0;
    m_sequence_ubo.flip_v         = info.flip_v ? 1 : 0;
}

TextureSequence::~TextureSequence() = default;

void TextureSequence::update(GPUCopyPass const& /*copy_pass*/, uint64_t const delta_time)
{
    m_delta_time += delta_time;
    while (m_delta_time > m_time_delay) {
        m_delta_time -= m_time_delay;
        ++m_sequence_ubo.frame;
        if (m_sequence_ubo.frame >= m_sequence_ubo.v_frames_count * m_sequence_ubo.h_frames_count) { m_sequence_ubo.frame = 0; }
    }
}

void TextureSequence::pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t const /*delta_time*/) const
{
    auto color_target = cmd_buffer.get_default_target();
    m_target->set_as_target(color_target);
    auto const render_pass = cmd_buffer.start_render_pass({color_target});

    render_pass->bind(*m_sequence_pipeline);
    render_pass->bind(*m_sequence_vertex_buffer);
    render_pass->bind(*m_index_buffer);
    render_pass->bind(*m_texture);

    render_pass->push_vertex_uniform_data(0, m_sequence_ubo);

    auto const blend_factor = static_cast<float>(m_delta_time) / m_time_delay;
    auto const color        = float4(1.0F, 1.0F, 1.0F, blend_factor);
    render_pass->push_fragment_uniform_data(0, color);

    render_pass->draw(*m_index_buffer);
}

void TextureSequence::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_vertex_buffer);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_target);

    render_pass.push_vertex_uniform_data(0, m_ubo);
    render_pass.push_fragment_uniform_data(0, m_color);
    render_pass.draw(*m_index_buffer);
}

void TextureSequence::recalculate_model_matrix()
{
    m_ubo.model_mat = mul(mul(m_scaling_mat, m_rotation_mat_z), m_translation_mat);
}

void TextureSequence::set_screen_rect(storm::FRect const& rect)
{
    m_ubo.view_proj_mat =
        float4x4::orthographic(projection(frustum(rect.left, rect.right, rect.bottom, rect.top, -1.0F, 1.0F), zclip::zero));
}

void TextureSequence::set_rect(storm::FRect const& rect)
{
    m_translation_mat = float4x4::translation(rect.left, rect.top, 0.0F);
    m_scaling_mat     = float4x4::scale(rect.width(), rect.height(), 1.0F);
    recalculate_model_matrix();
}

void TextureSequence::set_rotation(float angle)
{
    m_rotation_mat_z = float4x4::rotation_z(angle);
    recalculate_model_matrix();
}

void TextureSequence::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_color                 = float4(r, g, b, a);
}

auto TextureSequence::get_dimensions() const -> std::pair<uint32_t, uint32_t>
{
    return std::make_pair(m_width, m_height);
}
