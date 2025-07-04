#pragma once

#include <memory>
#include <string>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/types.h>

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCommandBuffer;
class GPUCopyPass;
class GPURenderPass;

class TextureSequence final
{
public:
    TextureSequence(GPUCopyPass const& copy_pass, std::string const& name);
    ~TextureSequence();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t delta_time) const;
    void draw(GPURenderPass const& render_pass) const;

    void set_rect(storm::FRect const& rect);
    void set_rotation(float angle);

    void set_screen_rect(storm::FRect const& rect);
    void set_diffuse_color(storm::Color const& color);

    auto get_dimensions() const -> std::pair<uint32_t, uint32_t>;

private:
    void recalculate_model_matrix();

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_time_delay;
    uint32_t m_delta_time;

    struct UBO {
        hlsl::float4x4 model_mat     = hlsl::float4x4::identity();
        hlsl::float4x4 view_proj_mat = hlsl::float4x4::identity();
    } m_ubo;

    struct alignas(16) SequenceUBO {
        int32_t frame;
        int32_t h_frames_count;
        int32_t v_frames_count;
        int16_t flip_h;
        int16_t flip_v;
    } m_sequence_ubo;

    hlsl::float4x4 m_translation_mat = hlsl::float4x4::identity();
    hlsl::float4x4 m_scaling_mat     = hlsl::float4x4::identity();

    hlsl::float4x4 m_rotation_mat_z = hlsl::float4x4::identity();

    hlsl::float4 m_color;

    std::unique_ptr<GraphicsPipeline> m_sequence_pipeline;
    std::unique_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture> m_target;
    std::shared_ptr<GPUTexture> m_texture;

    std::unique_ptr<GPUVertexBuffer> m_sequence_vertex_buffer;
    std::unique_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::unique_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
