#pragma once

#include <memory>

#include "hlslpp.h"
#include "i_post_processor.h"

namespace storm
{

class IBuffer;
class IIndexBuffer;
class IPipeline;
class ITexture;

class DrawTexture final: virtual public IPostProcessor
{
public:
    explicit DrawTexture();

    void update(uint64_t delta_time) override;
    void render(ITexture& scene_target) const override;
    void set_next(std::shared_ptr<IPostProcessor> const& next) override;

    std::shared_ptr<IPostProcessor> get_next() const override;

private:
    hlsl::float4x4 m_view_proj_matrix = hlsl::float4x4::identity();
    hlsl::float4   m_color;

    std::shared_ptr<IPipeline> m_pipeline = nullptr;

    std::shared_ptr<IBuffer>      m_vertex_buffer = nullptr;
    std::shared_ptr<IIndexBuffer> m_index_buffer  = nullptr;
};

}  // namespace storm
