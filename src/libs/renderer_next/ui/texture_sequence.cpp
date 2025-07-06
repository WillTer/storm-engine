#include "texture_sequence.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/config/texture_sequence.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <libs/renderer_next/pipeline_names.h>
#include <shaders/ui/texture_sequence.h>

using namespace storm;
using namespace hlslpp;

using Vertex = shaders::texture_sequence::VertexInput;

namespace
{

auto const SQUARE_VERTICES = std::vector<Vertex> {
    Vertex {{-1.0F, -1.0F, 0.0F, 1.0F}},
    Vertex {{1.0F, -1.0F, 0.0F, 1.0F}},
    Vertex {{1.0F, 1.0F, 0.0F, 1.0F}},
    Vertex {{-1.0F, 1.0F, 0.0F, 1.0F}},
};

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

}  // namespace

TextureSequence::TextureSequence(std::string const& name)
{
    auto const& renderer      = core->get<RendererService>();
    auto const& config_loader = core->get<IConfigLoader>();

    auto const info = storm::texture_sequence::info(*config_loader, name);

    m_texture = renderer->create_texture(info.texture_file);
    m_target  = renderer->create_texture_target(info.width, info.height);

    m_pipeline = renderer->create_pipeline(TEXTURE_SEQUENCE_PIPELINE);

    m_vertex_buffer = renderer->create_vertex_buffer(SQUARE_VERTICES);
    m_index_buffer  = renderer->create_index_buffer(SQUARE_INDICES);

    m_delta_time = 0;
    m_time_delay = info.time_delay;

    m_vertex_ubo.frame          = 0;
    m_vertex_ubo.h_frames_count = info.h_frames_count;
    m_vertex_ubo.v_frames_count = info.v_frames_count;
    m_vertex_ubo.flip_h         = info.flip_h ? 1 : 0;
    m_vertex_ubo.flip_v         = info.flip_v ? 1 : 0;
}

TextureSequence::~TextureSequence() = default;

void TextureSequence::update(GPUCopyPass const& /*copy_pass*/, uint64_t const delta_time)
{
    m_delta_time += delta_time;
    while (m_delta_time > m_time_delay) {
        m_delta_time -= m_time_delay;
        ++m_vertex_ubo.frame;
        if (m_vertex_ubo.frame >= m_vertex_ubo.v_frames_count * m_vertex_ubo.h_frames_count) { m_vertex_ubo.frame = 0; }
    }
}

void TextureSequence::pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t const /*delta_time*/) const
{
    auto color_target = cmd_buffer.get_default_target();
    m_target->set_as_target(color_target);
    auto const render_pass = cmd_buffer.start_render_pass({color_target});

    render_pass->bind(*m_pipeline);
    render_pass->bind(*m_vertex_buffer);
    render_pass->bind(*m_index_buffer);
    render_pass->bind(*m_texture);

    render_pass->push_vertex_uniform_data(0, m_vertex_ubo);

    auto const blend_factor = static_cast<float>(m_delta_time) / m_time_delay;
    auto const color        = float4(1.0F, 1.0F, 1.0F, blend_factor);
    render_pass->push_fragment_uniform_data(0, color);

    render_pass->draw(*m_index_buffer);
}

auto TextureSequence::get_target_texture() const -> std::shared_ptr<GPUTexture>
{
    return m_target;
}
