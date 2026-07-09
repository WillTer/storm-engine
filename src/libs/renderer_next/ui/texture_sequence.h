#pragma once

#include <memory>

#include <libs/config/texture_sequence.h>
#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/i_pre_drawable.h>
#include <libs/renderer_next/types.h>
#include <shaders/ubo_types.h>

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCommandBuffer;
class GPUCopyPass;
class GPURenderPass;

class RendererService;

class TextureSequence final: virtual public IPreDrawable
{
public:
    TextureSequence(std::shared_ptr<RendererService> const& renderer, TextureSequenceInfo const& info);
    ~TextureSequence() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t delta_time) const override;

    void next_frame();

    auto get_target_texture() const -> std::shared_ptr<GPUTexture>;

private:
    uint32_t m_time_delay;
    uint64_t m_delta_time;

    shaders::AnimationUBOVertex m_vertex_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture> m_target;
    std::shared_ptr<GPUTexture> m_texture;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
