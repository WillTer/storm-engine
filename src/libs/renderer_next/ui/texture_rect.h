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

class TextureRect
{
public:
    explicit TextureRect(GPUCopyPass const& copy_pass, std::filesystem::path const& texture);
    virtual ~TextureRect();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_rect(storm::FRect const& rect);
    void set_screen_rect(storm::FRect const& rect);

private:
    struct UBO {
        hlsl::float4x4 model_mat     = hlsl::float4x4::identity();
        hlsl::float4x4 view_proj_mat = hlsl::float4x4::identity();
    } m_ubo;

    std::unique_ptr<GraphicsPipeline> m_pipeline;

    std::unique_ptr<GPUTexture>      m_texture;
    std::unique_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::unique_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
