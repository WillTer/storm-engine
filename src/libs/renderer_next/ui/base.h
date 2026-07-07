#pragma once

#include <memory>
#include <vector>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/i_drawable.h>
#include <libs/renderer_next/types.h>
#include <shaders/ubo_types.h>

namespace storm
{
class GraphicsPipeline;
class GPUIndexBuffer;
class GPUVertexBuffer;
class GPUTexture;
}  // namespace storm

namespace storm::renderer::ui
{

class Base: virtual public IDrawable
{
public:
    Base();
    virtual ~Base();

    virtual void set_rect(storm::FRect const& rect);
    virtual void set_rotation(float angle);

    virtual void set_screen_rect(storm::FRect const& rect);

    virtual auto get_dimensions() const -> std::pair<uint32_t, uint32_t>;
    virtual auto get_rect() const -> storm::FRect const&;

    virtual void set_pipeline(std::shared_ptr<GraphicsPipeline> const& pipeline);

    virtual void create_default_pipeline(std::string const& fragment_shader = {})                   = 0;
    virtual void set_technique(std::string const& technique, std::string const& vertex_shader = {}) = 0;

protected:
    shaders::UBOVertex   m_vertex_ubo;
    shaders::UBOFragment m_fragment_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;
    std::shared_ptr<GPUIndexBuffer>   m_index_buffer;
    std::shared_ptr<GPUVertexBuffer>  m_vertex_buffer;

private:
    void recalculate_model_matrix();

    uint32_t m_width  = 0;
    uint32_t m_height = 0;

    storm::FRect        m_rect = {};
    std::vector<float4> m_colors;

    hlsl::float4x4 m_translation_mat = hlsl::float4x4::identity();
    hlsl::float4x4 m_scaling_mat     = hlsl::float4x4::identity();
    hlsl::float4x4 m_rotation_mat_z  = hlsl::float4x4::identity();
};

}  // namespace storm::renderer::ui
