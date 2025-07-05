#pragma once

#include <memory>
#include <string>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/types.h>
#include <shaders/ui/ubo_types.h>

#include "image_2d_base.h"

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCommandBuffer;
class GPUCopyPass;
class GPURenderPass;

class TextureSequence final: public Image2DBase
{
public:
    TextureSequence(GPUCopyPass const& copy_pass, std::string const& name);
    ~TextureSequence() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t delta_time) const;
    void draw(GPURenderPass const& render_pass) const;

    void set_diffuse_color(storm::Color const& color);

private:
    uint32_t m_time_delay;
    uint64_t m_delta_time;

    shaders::AnimationUBOVertex m_sequence_ubo;
    shaders::UBOFragment        m_fragment_ubo;

    std::unique_ptr<GraphicsPipeline> m_sequence_pipeline;
    std::unique_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture> m_target;
    std::shared_ptr<GPUTexture> m_texture;

    std::unique_ptr<GPUVertexBuffer> m_sequence_vertex_buffer;
    std::unique_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::unique_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
