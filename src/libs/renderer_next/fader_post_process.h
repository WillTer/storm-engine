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
class IScene;

class FaderPostProcess final: virtual public IPostProcessor
{
public:
    explicit FaderPostProcess();

    void update(uint64_t delta_time) override;
    void render(ITexture& scene_target) const override;
    void set_next(std::shared_ptr<IPostProcessor> const& next) override;

    std::shared_ptr<IPostProcessor> get_next() const override;

    void start_fade(float alpha, float speed);
    bool is_fade_finished() const;

private:
    hlsl::float4x4 m_view_proj_matrix = hlsl::float4x4::identity();
    hlsl::float4   m_color;

    std::shared_ptr<IPipeline> m_pipeline = nullptr;

    std::shared_ptr<ITextureTarget> m_render_target = nullptr;
    std::shared_ptr<IPostProcessor> m_next          = nullptr;

    std::shared_ptr<IBuffer>      m_vertex_buffer = nullptr;
    std::shared_ptr<IIndexBuffer> m_index_buffer  = nullptr;

    float m_fade_alpha = 1.0F;
    float m_fade_speed = 1.0F;

    bool m_is_fade_finished = true;
};

}  // namespace storm
