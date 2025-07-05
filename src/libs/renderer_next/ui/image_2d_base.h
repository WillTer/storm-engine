#pragma once

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/types.h>
#include <shaders/ui/ubo_types.h>

namespace storm
{

class Image2DBase
{
public:
    Image2DBase(storm::FRect const& rect = {});
    virtual ~Image2DBase();

    virtual void set_rect(storm::FRect const& rect);
    virtual void set_rotation(float angle);

    virtual void set_screen_rect(storm::FRect const& rect);

    virtual auto get_dimensions() const -> std::pair<uint32_t, uint32_t>;
    virtual auto get_rect() const -> storm::FRect;

protected:
    void recalculate_model_matrix();

    uint32_t     m_width  = 0;
    uint32_t     m_height = 0;
    storm::FRect m_rect   = {};

    shaders::UBOVertex m_vertex_ubo;

    hlsl::float4x4 m_translation_mat = hlsl::float4x4::identity();
    hlsl::float4x4 m_scaling_mat     = hlsl::float4x4::identity();
    hlsl::float4x4 m_rotation_mat_z  = hlsl::float4x4::identity();
};

}  // namespace storm
