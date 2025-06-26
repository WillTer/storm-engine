#pragma once

#include <memory>

#include <libs/config/main_config.h>

#include "hlslpp.h"

namespace storm
{

class IBuffer;
class IIndexBuffer;
class IPipeline;
class ITexture;
class AssetServer;
class RendererNext;

class ProgressImageView final
{
public:
    explicit ProgressImageView(
        std::shared_ptr<AssetServer> const&   asset_server,
        std::shared_ptr<IConfigLoader> const& config_loader,
        std::shared_ptr<RendererNext> const&  renderer);

    void set_picture(std::shared_ptr<ITexture> const& image);
    void set_background(std::shared_ptr<ITexture> const& image);

    void update(uint64_t delta_time);
    void present() const;

    void set_fade_speed(float speed);
    void set_fade_alpha(float alpha);

private:
    void process_fader(uint64_t delta_time);
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

    std::shared_ptr<IBuffer>      m_vertex_buffer_back     = nullptr;
    std::shared_ptr<IBuffer>      m_vertex_buffer_progress = nullptr;
    std::shared_ptr<IIndexBuffer> m_index_buffer           = nullptr;

    UBO m_progress_ubo;
    UBO m_picture_ubo;
    UBO m_background_ubo;

    ProgressImageInfo m_progress_info;

    float m_fade_alpha = 1.0F;
    float m_fade_speed = 1.0F;

    uint32_t m_current_frame = 0;
};

}  // namespace storm
