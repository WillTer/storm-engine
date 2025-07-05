#include "image_2d_base.h"

#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

using namespace storm;
using namespace hlslpp;

Image2DBase::Image2DBase(storm::FRect const& rect /*= {}*/) : m_rect(rect)
{
    m_width  = static_cast<uint32_t>(m_rect.width());
    m_height = static_cast<uint32_t>(m_rect.height());

    auto const& renderer = core->get<RendererService>();
    auto const  viewport = renderer->get_viewport();
    set_screen_rect(viewport);  // Use viewport rect for projection matrix by default

    m_vertex_ubo.model     = float4x4::identity();
    m_vertex_ubo.view_proj = float4x4::identity();
}

Image2DBase::~Image2DBase() = default;

void Image2DBase::recalculate_model_matrix()
{
    m_vertex_ubo.model = mul(mul(m_scaling_mat, m_rotation_mat_z), m_translation_mat);
}

void Image2DBase::set_screen_rect(storm::FRect const& rect)
{
    m_vertex_ubo.view_proj =
        float4x4::orthographic(projection(frustum(rect.left, rect.right, rect.bottom, rect.top, -1.0F, 1.0F), zclip::zero));
}

void Image2DBase::set_rect(storm::FRect const& rect)
{
    m_rect            = rect;
    m_translation_mat = float4x4::translation(rect.left, rect.top, 0.0F);
    m_scaling_mat     = float4x4::scale(rect.width(), rect.height(), 1.0F);
    recalculate_model_matrix();
}

void Image2DBase::set_rotation(float angle)
{
    m_rotation_mat_z = float4x4::rotation_z(angle);
    recalculate_model_matrix();
}

auto Image2DBase::get_dimensions() const -> std::pair<uint32_t, uint32_t>
{
    return std::make_pair(m_width, m_height);
}

auto Image2DBase::get_rect() const -> storm::FRect
{
    return m_rect;
}
