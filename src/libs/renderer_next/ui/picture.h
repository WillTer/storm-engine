#pragma once

#include <filesystem>
#include <memory>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/types.h>

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCopyPass;
class GPURenderPass;

class Picture
{
public:
    Picture(GPUCopyPass const& copy_pass, std::filesystem::path const& texture, storm::FRect const& texture_rect = default_texture_rect());
    Picture(
        GPUCopyPass const&                 copy_pass,
        std::shared_ptr<GPUTexture> const& external_texture,
        storm::FRect const&                texture_rect = default_texture_rect());

    virtual ~Picture();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_rect(storm::FRect const& rect);
    void set_rotation(float angle);

    void set_screen_rect(storm::FRect const& rect);
    void set_diffuse_color(storm::Color const& color);

    auto get_dimensions() const -> std::pair<uint32_t, uint32_t>;

private:
    void initialize(GPUCopyPass const& copy_pass, storm::FRect const& texture_rect);
    void recalculate_model_matrix();

    static constexpr storm::FRect default_texture_rect()
    {
        return storm::FRect {
            .left   = 0.0F,
            .top    = 0.0F,
            .right  = 1.0F,
            .bottom = 1.0F,
        };
    }

    uint32_t m_width  = 0;
    uint32_t m_height = 0;

    struct UBO {
        hlsl::float4x4 model_mat     = hlsl::float4x4::identity();
        hlsl::float4x4 view_proj_mat = hlsl::float4x4::identity();
    } m_ubo;

    hlsl::float4x4 m_translation_mat = hlsl::float4x4::identity();
    hlsl::float4x4 m_scaling_mat     = hlsl::float4x4::identity();

    hlsl::float4x4 m_rotation_mat_z = hlsl::float4x4::identity();

    hlsl::float4 m_color;

    std::unique_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture>      m_texture;
    std::unique_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::unique_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
