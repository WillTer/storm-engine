#pragma once

#include <memory>

#include <libs/config/main_config.h>
#include <shaders/ui/ubo_types.h>

namespace storm
{

class RendererService;

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPURenderPass;
class GPUCopyPass;

class ProgressImageScene final
{
public:
    explicit ProgressImageScene(std::shared_ptr<IConfigLoader> const& config_loader, std::shared_ptr<RendererService> const& renderer);

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_picture(std::shared_ptr<GPUTexture> const& image);
    void set_background(std::shared_ptr<GPUTexture> const& image);

private:
    void process_progress(GPUCopyPass const& copy_pass);

    void update_picture_matrices();
    void update_progress_matrices();

    std::shared_ptr<GraphicsPipeline> m_pipeline = nullptr;

    std::shared_ptr<GPUTexture> m_progress   = nullptr;
    std::shared_ptr<GPUTexture> m_picture    = nullptr;
    std::shared_ptr<GPUTexture> m_frame      = nullptr;
    std::shared_ptr<GPUTexture> m_background = nullptr;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_back     = nullptr;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_progress = nullptr;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer           = nullptr;

    shaders::UBOVertex m_progress_ubo;
    shaders::UBOVertex m_picture_ubo;
    shaders::UBOVertex m_background_ubo;

    ProgressImageInfo m_progress_info;

    uint32_t m_current_frame = 0;
};

}  // namespace storm
