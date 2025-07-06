#pragma once

#include <memory>
#include <string>
#include <vector>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/i_pre_drawable.h>
#include <libs/renderer_next/types.h>
#include <shaders/ui/ubo_types.h>

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCommandBuffer;
class GPUCopyPass;
class GPURenderPass;

class TextureSequence final: virtual public IPreDrawable
{
public:
    TextureSequence(std::string const& name);
    ~TextureSequence() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void pre_draw(GPUCommandBuffer const& cmd_buffer, uint64_t delta_time) const override;

    auto get_target_texture() const -> std::shared_ptr<GPUTexture>;

private:
    bool m_need_upload;

    struct UploadData {
        std::vector<char> texture_data;
    } m_upload_data;

    uint32_t m_time_delay;
    uint64_t m_delta_time;

    shaders::AnimationUBOVertex m_vertex_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture> m_target;
    std::shared_ptr<GPUTexture> m_texture;

    std::unique_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::unique_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
