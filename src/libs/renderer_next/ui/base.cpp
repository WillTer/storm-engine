#include "base.h"

#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_copy_pass.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

using namespace storm;
using namespace storm::renderer::ui;
using namespace hlslpp;

Base::Base()
{
    auto const& renderer = core->get<RendererService>();
    auto const  viewport = renderer->get_viewport();
    set_screen_rect(viewport);  // Use viewport rect for projection matrix by default

    m_vertex_ubo.model     = float4x4::identity();
    m_vertex_ubo.view_proj = float4x4::identity();

    m_fragment_ubo.diffuse = float4(1.0F);
}

Base::~Base() = default;

void Base::recalculate_model_matrix()
{
    m_vertex_ubo.model = mul(mul(m_scaling_mat, m_rotation_mat_z), m_translation_mat);
}

void Base::set_screen_rect(storm::FRect const& rect)
{
    m_vertex_ubo.view_proj =
        float4x4::orthographic(projection(frustum(rect.left, rect.right, rect.bottom, rect.top, -1.0F, 1.0F), zclip::zero));
}

void Base::set_rect(storm::FRect const& rect)
{
    m_rect   = rect;
    m_width  = static_cast<uint32_t>(m_rect.width());
    m_height = static_cast<uint32_t>(m_rect.height());

    m_translation_mat = float4x4::translation(rect.left, rect.top, 0.0F);
    m_scaling_mat     = float4x4::scale(rect.width(), rect.height(), 1.0F);
    recalculate_model_matrix();
}

void Base::set_rotation(float angle)
{
    m_rotation_mat_z = float4x4::rotation_z(angle);
    recalculate_model_matrix();
}

auto Base::get_dimensions() const -> std::pair<uint32_t, uint32_t>
{
    return std::make_pair(m_width, m_height);
}

auto Base::get_rect() const -> storm::FRect const&
{
    return m_rect;
}

void Base::set_pipeline(std::shared_ptr<GraphicsPipeline> const& pipeline)
{
    m_pipeline = pipeline;
}
