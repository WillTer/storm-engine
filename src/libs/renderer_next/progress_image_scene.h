#pragma once

#include <memory>

#include <libs/config/main_config.h>

#include "hlslpp.h"

namespace storm
{

class AssetServer;
class RendererService;

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;

class ProgressImageScene final
{
public:
    explicit ProgressImageScene(
        std::shared_ptr<AssetServer> const&     asset_server,
        std::shared_ptr<IConfigLoader> const&   config_loader,
        std::shared_ptr<RendererService> const& renderer);

    void update(uint64_t delta_time);
    void render() const;

    void set_picture(std::shared_ptr<GPUTexture> const& image);
    void set_background(std::shared_ptr<GPUTexture> const& image);

private:
    void process_progress();

    void update_picture_matrices();
    void update_progress_matrices();

    struct UBO {
        hlsl::float4x4 m_model_matrix     = hlsl::float4x4::identity();
        hlsl::float4x4 m_view_proj_matrix = hlsl::float4x4::identity();
    };

    std::shared_ptr<GraphicsPipeline> m_pipeline = nullptr;

    std::shared_ptr<GPUTexture> m_progress   = nullptr;
    std::shared_ptr<GPUTexture> m_picture    = nullptr;
    std::shared_ptr<GPUTexture> m_frame      = nullptr;
    std::shared_ptr<GPUTexture> m_background = nullptr;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_back     = nullptr;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_progress = nullptr;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer           = nullptr;

    UBO m_progress_ubo;
    UBO m_picture_ubo;
    UBO m_background_ubo;

    ProgressImageInfo m_progress_info;

    uint32_t m_current_frame = 0;
};

}  // namespace storm
