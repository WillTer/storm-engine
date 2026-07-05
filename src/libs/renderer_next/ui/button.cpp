#include "button.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <shaders/ui/image_2d.h>

using namespace storm;
using namespace hlslpp;

using ImageVertex = shaders::image_2d::VertexInput;

namespace
{

auto const SQUARE_INDICES = std::vector<uint32_t> {0, 1, 2, 0, 2, 3};

}  // namespace

Button::Button(
    std::shared_ptr<GPUTexture> const& texture,
    storm::FRect const&                tex_rect_left,
    storm::FRect const&                tex_rect_middle,
    storm::FRect const&                tex_rect_right,
    storm::FRect const&                button_rect)
    : m_texture(texture)
{
    auto const& renderer = core->get<RendererService>();

    m_pipeline = renderer->create_pipeline<ImageVertex>("ui/image_2d", "ui/tex_ubo_diffuse");

    auto const [width, height] = m_texture->get_dimensions();

    // Get relative width of left and right parts
    auto const left_width  = std::fabs((tex_rect_left.width() * width) / button_rect.width());
    auto const right_width = std::fabs((tex_rect_right.width() * width) / button_rect.width());

    // Left part - from 0 to left_width
    auto const vertex_data_left = std::vector<ImageVertex> {
        ImageVertex {{0.0F, 0.0F}, {tex_rect_left.left, tex_rect_left.top}},
        ImageVertex {{left_width, 0.0F}, {tex_rect_left.right, tex_rect_left.top}},
        ImageVertex {{left_width, 1.0F}, {tex_rect_left.right, tex_rect_left.bottom}},
        ImageVertex {{0.0F, 1.0F}, {tex_rect_left.left, tex_rect_left.bottom}},
    };

    // Middle part - from left_width to 1 - right_width
    auto const vertex_data_middle = std::vector<ImageVertex> {
        ImageVertex {{left_width, 0.0F}, {tex_rect_middle.left, tex_rect_middle.top}},
        ImageVertex {{1.0F - right_width, 0.0F}, {tex_rect_middle.right, tex_rect_middle.top}},
        ImageVertex {{1.0F - right_width, 1.0F}, {tex_rect_middle.right, tex_rect_middle.bottom}},
        ImageVertex {{left_width, 1.0F}, {tex_rect_middle.left, tex_rect_middle.bottom}},
    };

    // Right part - from 1 - right_width to 1
    auto const vertex_data_right = std::vector<ImageVertex> {
        ImageVertex {{1.0F - right_width, 0.0F}, {tex_rect_right.left, tex_rect_right.top}},
        ImageVertex {{1.0F, 0.0F}, {tex_rect_right.right, tex_rect_right.top}},
        ImageVertex {{1.0F, 1.0F}, {tex_rect_right.right, tex_rect_right.bottom}},
        ImageVertex {{1.0F - right_width, 1.0F}, {tex_rect_right.left, tex_rect_right.bottom}},
    };

    m_vertex_buffer_left   = renderer->create_vertex_buffer(vertex_data_left);
    m_vertex_buffer_middle = renderer->create_vertex_buffer(vertex_data_middle);
    m_vertex_buffer_right  = renderer->create_vertex_buffer(vertex_data_right);
    m_index_buffer         = renderer->create_index_buffer(SQUARE_INDICES);

    m_fragment_ubo.diffuse = float4(1.0F);

    m_rect   = button_rect;
    m_width  = static_cast<uint32_t>(m_rect.width());
    m_height = static_cast<uint32_t>(m_rect.height());

    m_middle_rect = {
        .left   = button_rect.left + (tex_rect_left.width() * width),
        .top    = button_rect.top,
        .right  = button_rect.right - (tex_rect_right.width() * width),
        .bottom = button_rect.bottom,
    };
}

Button::~Button() = default;

void Button::update(GPUCopyPass const& /*copy_pass*/, uint64_t const /*delta_time*/) {}

void Button::draw(GPURenderPass const& render_pass) const
{
    render_pass.bind(*m_pipeline);
    render_pass.bind(*m_index_buffer);
    render_pass.bind(*m_texture);
    render_pass.push_vertex_uniform_data(0, m_vertex_ubo);
    render_pass.push_fragment_uniform_data(0, m_fragment_ubo);

    render_pass.bind(*m_vertex_buffer_middle);
    render_pass.draw(*m_index_buffer);

    render_pass.bind(*m_vertex_buffer_left);
    render_pass.draw(*m_index_buffer);

    render_pass.bind(*m_vertex_buffer_right);
    render_pass.draw(*m_index_buffer);
}

void Button::set_diffuse_color(storm::Color const& color)
{
    auto const [r, g, b, a] = color.normalize();
    m_fragment_ubo.diffuse  = float4(r, g, b, a);
}

auto Button::get_middle_rect() const -> storm::FRect
{
    return m_middle_rect;
}
