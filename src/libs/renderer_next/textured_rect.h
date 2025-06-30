#pragma once

#include <filesystem>
#include <memory>

#include "hlslpp.h"
#include "types.h"

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCopyPass;
class GPURenderPass;

class TexturedRect
{
public:
    explicit TexturedRect(GPUCopyPass const& copy_pass, std::filesystem::path const& texture);
    virtual ~TexturedRect();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_screen_rect(storm::FRect const& rect);

    void set_position(storm::FPoint const& pos);
    void set_size(float width, float height);
    void set_diffuse_color(storm::Color const& color);

private:
    struct UBO {
        hlsl::float4x4 model_mat     = hlsl::float4x4::identity();
        hlsl::float4x4 view_proj_mat = hlsl::float4x4::identity();
    } m_ubo;

    hlsl::float4x4 m_translation = hlsl::float4x4::identity();
    hlsl::float4x4 m_scale       = hlsl::float4x4::identity();

    hlsl::float4 m_color;

    std::shared_ptr<GraphicsPipeline> m_pipeline = nullptr;

    std::shared_ptr<GPUTexture>      m_texture       = nullptr;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer = nullptr;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer  = nullptr;
};

}  // namespace storm
