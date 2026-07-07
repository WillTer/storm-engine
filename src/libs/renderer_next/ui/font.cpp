#include "font.h"

#include <libs/asset_server/asset_server.h>
#include <libs/config/main_config.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <shaders/ui/font_normal.h>

#include "libs/renderer_next/impl_sdl/gpu_command_buffer.h"

using namespace storm;
using namespace hlslpp;

using Vertex = shaders::font_normal::VertexInput;

namespace
{

auto const SQUARE_VERTICES = std::vector<Vertex> {
    Vertex {{0.0F, 0.0F}},
    Vertex {{1.0F, 0.0F}},
    Vertex {{1.0F, 1.0F}},
    Vertex {{0.0F, 1.0F}},
};

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

}  // namespace

Font::Font(std::string const& font_name)
{
    auto const& renderer      = core->get<RendererService>();
    auto const& config_loader = core->get<IConfigLoader>();

    auto const window_info = storm::main_config::window_info(*config_loader);

    m_info    = storm::font::info(*config_loader, window_info.font_config, font_name);
    m_texture = renderer->create_texture(m_info.texture);

    if (m_info.technique.empty()) {
        m_pipeline = renderer->create_pipeline<Vertex>("ui/font_normal", "ui/picture");
    } else {
        auto const technique = renderer->get_technique_info(m_info.technique);
        m_pipeline           = renderer->create_pipeline<Vertex>("ui/font_normal", technique);
    }

    m_vertex_buffer = renderer->create_vertex_buffer(SQUARE_VERTICES);
    m_index_buffer  = renderer->create_index_buffer(SQUARE_INDICES);

    m_vertex_ubo.model      = float4x4::identity();
    m_vertex_ubo.view_proj  = float4x4::identity();
    m_vertex_ubo.tex_coords = float4(0.0F);

    m_fragment_ubo.diffuse = float4(1.0F);
}

Font::~Font() = default;

float Font::get_string_width(std::string const& text, std::optional<float> scale_override) const
{
    if (text.empty()) {
        return 0;
    }

    float       width = 0;
    float const scale = scale_override.value_or(1.0F) * m_info.pc_scale;

    for (size_t i = 0; i < text.size(); i += utf8::u8_inc(text.data() + i)) {
        uint32_t codepoint = utf8::Utf8ToCodepoint(text.data() + i);

        if (!m_info.symbols.contains(codepoint)) {
            codepoint = ' ';
        }

        if (codepoint == ' ') {
            width += (m_info.spacebar * scale) + (m_info.symbol_interval * scale);
            continue;
        }

        auto const& rect = m_info.symbols.at(codepoint);
        width += (rect.width() * m_info.texture_width * scale) + (m_info.symbol_interval * scale);
    }

    return width;
}

auto Font::print(
    GPUCommandBuffer const& cmd_buffer,
    uint32_t                fg,
    uint32_t                bg,
    Alignment               align,
    bool                    draw_shadow,
    float                   scale,
    float                   width,
    float                   height,
    std::string const&      text) -> std::shared_ptr<GPUTexture>
{
    return print(cmd_buffer, Color::from_hex(fg), Color::from_hex(bg), align, draw_shadow, scale, width, height, text);
}

auto Font::print(
    GPUCommandBuffer const& cmd_buffer,
    Color                   fg,
    Color                   bg,
    Alignment               align,
    bool                    draw_shadow,
    float                   scale,
    float                   width,
    float                   height,
    std::string const&      text) -> std::shared_ptr<GPUTexture>
{
    auto const& renderer       = core->get<RendererService>();
    auto        target_texture = renderer->create_texture_target(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    m_vertex_ubo.model     = float4x4::identity();
    m_vertex_ubo.view_proj = float4x4::orthographic(projection(frustum(0.0F, width, height, 0.0F, -1.0F, 1.0F), zclip::zero));

    auto const [fg_r, fg_g, fg_b, fg_a] = fg.normalize();
    auto const fg_color                 = float4(fg_r, fg_g, fg_b, fg_a);

    auto const [bg_r, bg_g, bg_b, bg_a] = bg.normalize();
    auto const shadow_color             = float4(bg_r, bg_g, bg_b, 1.0F);

    auto const str_width = get_string_width(text, scale);

    auto const total_scale = scale * m_info.pc_scale;
    auto const str_height  = m_info.height * total_scale;

    auto const offset_y = (height - str_height) / 2.0F;
    auto       offset_x = 0.0F;
    switch (align) {
    case Alignment::Left: break;
    case Alignment::Center: offset_x = (width - str_width) / 2.0F; break;
    case Alignment::Right: offset_x = width - str_width; break;
    }

    auto color_target        = cmd_buffer.get_default_target();
    color_target.clear_color = bg.normalize();

    target_texture->set_as_target(color_target);

    auto const render_pass = cmd_buffer.start_render_pass({color_target});
    render_pass->bind(*m_pipeline);
    render_pass->bind(*m_vertex_buffer);
    render_pass->bind(*m_index_buffer);
    render_pass->bind(*m_texture);

    for (size_t i = 0; i < text.size(); i += utf8::u8_inc(text.data() + i)) {
        uint32_t codepoint = utf8::Utf8ToCodepoint(text.data() + i);

        if (!m_info.symbols.contains(codepoint)) {
            codepoint = ' ';
        }

        if (codepoint == ' ') {
            offset_x += (m_info.spacebar * total_scale) + (m_info.symbol_interval * total_scale);
            continue;
        }

        auto const& rect        = m_info.symbols.at(codepoint);
        m_vertex_ubo.tex_coords = float4(rect.left, rect.top, rect.right, rect.bottom);

        auto const translation_mat = float4x4::translation(offset_x, offset_y, 0.0F);
        auto const scaling_mat =
            float4x4::scale(rect.width() * m_info.texture_width * total_scale, rect.height() * m_info.texture_height * total_scale, 1.0F);

        if (draw_shadow) {
            m_vertex_ubo.model =
                mul(scaling_mat, float4x4::translation(offset_x + m_info.shadow_offset_x, offset_y + m_info.shadow_offset_y, 0.0F));
            m_fragment_ubo.diffuse = shadow_color;

            render_pass->push_vertex_uniform_data(0, m_vertex_ubo);
            render_pass->push_fragment_uniform_data(0, m_fragment_ubo);
            render_pass->draw(*m_index_buffer);
        }

        m_vertex_ubo.model     = mul(scaling_mat, translation_mat);
        m_fragment_ubo.diffuse = fg_color;

        render_pass->push_vertex_uniform_data(0, m_vertex_ubo);
        render_pass->push_fragment_uniform_data(0, m_fragment_ubo);
        render_pass->draw(*m_index_buffer);

        offset_x += (rect.width() * m_info.texture_width * total_scale) + (m_info.symbol_interval * total_scale);
    }

    return target_texture;
}
