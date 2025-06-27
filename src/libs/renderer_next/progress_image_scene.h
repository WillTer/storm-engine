#pragma once

#include <memory>
#include <vector>

#include <libs/config/main_config.h>

#include "hlslpp.h"
#include "i_scene.h"

namespace storm
{

class IBuffer;
class IIndexBuffer;
class IPipeline;
class ITexture;
class AssetServer;
class RendererNext;

class ProgressImageScene final: virtual public IScene
{
public:
    explicit ProgressImageScene(
        std::shared_ptr<AssetServer> const&   asset_server,
        std::shared_ptr<IConfigLoader> const& config_loader,
        std::shared_ptr<RendererNext> const&  renderer);

    void update(uint64_t delta_time) override;
    void render() const override;

    void set_post_processor(std::shared_ptr<IPostProcessor> const& post_processor) override;

    std::shared_ptr<IPostProcessor> get_post_processor() const override;

    void set_picture(std::shared_ptr<ITexture> const& image);
    void set_background(std::shared_ptr<ITexture> const& image);

private:
    void process_progress();

    void update_picture_matrices();
    void update_progress_matrices();

    struct UBO {
        hlsl::float4x4 m_model_matrix     = hlsl::float4x4::identity();
        hlsl::float4x4 m_view_proj_matrix = hlsl::float4x4::identity();
    };

    std::shared_ptr<IPipeline> m_pipeline = nullptr;

    std::shared_ptr<ITexture> m_progress   = nullptr;
    std::shared_ptr<ITexture> m_picture    = nullptr;
    std::shared_ptr<ITexture> m_frame      = nullptr;
    std::shared_ptr<ITexture> m_background = nullptr;

    std::shared_ptr<ITextureTarget> m_render_target = nullptr;

    std::shared_ptr<IBuffer>      m_vertex_buffer_back     = nullptr;
    std::shared_ptr<IBuffer>      m_vertex_buffer_progress = nullptr;
    std::shared_ptr<IIndexBuffer> m_index_buffer           = nullptr;

    std::shared_ptr<IPostProcessor> m_post_processor;

    UBO m_progress_ubo;
    UBO m_picture_ubo;
    UBO m_background_ubo;

    ProgressImageInfo m_progress_info;

    uint32_t m_current_frame = 0;
};

}  // namespace storm
