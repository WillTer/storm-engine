#pragma once

#include <memory>

#include "hlslpp.h"
#include "i_texture_drawer.h"

namespace storm
{

class IBuffer;
class IIndexBuffer;
class IPipeline;
class ITexture;

class DrawTexture: virtual public ITextureDrawer
{
public:
    explicit DrawTexture();
    ~DrawTexture() override;

    void update(uint64_t delta_time) override;
    void present(ITexture& source) const override;

protected:
    hlsl::float4x4 m_view_proj_matrix = hlsl::float4x4::identity();
    hlsl::float4   m_color;

private:
    std::shared_ptr<IPipeline> m_pipeline = nullptr;

    std::shared_ptr<IBuffer>      m_vertex_buffer = nullptr;
    std::shared_ptr<IIndexBuffer> m_index_buffer  = nullptr;
};

}  // namespace storm
