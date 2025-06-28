#pragma once

#include <memory>

#include "hlslpp.h"

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;

class DrawTexture
{
public:
    explicit DrawTexture();
    virtual ~DrawTexture();

    void update(uint64_t delta_time);
    void present(GPUTexture& source) const;

protected:
    hlsl::float4x4 m_view_proj_matrix = hlsl::float4x4::identity();
    hlsl::float4   m_color;

private:
    std::shared_ptr<GraphicsPipeline> m_pipeline = nullptr;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer = nullptr;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer  = nullptr;
};

}  // namespace storm
