#pragma once

#include <memory>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#pragma warning(push)
#pragma warning(disable : 4201)  // warning C4201: nonstandard extension used: nameless struct/union
#pragma warning(disable : 4324)  // warning C4324: structure was padded due to alignment specifier
#include <glm/mat4x4.hpp>
#pragma warning(pop)

#include <libs/config/main_config.h>

namespace storm
{

class IBuffer;
class IIndexBuffer;
class IPipeline;
class ITexture;

class ProgressImageView final
{
public:
    explicit ProgressImageView();

    void set_background(std::shared_ptr<ITexture> const& image);

    void update(uint64_t delta_time);
    void present() const;

    void set_fade_speed(float speed);

private:
    struct UBO {
        glm::mat4 m_model_matrix     = glm::mat4(1.0F);
        glm::mat4 m_view_proj_matrix = glm::mat4(1.0F);
    };

    std::shared_ptr<IPipeline> m_pipeline = nullptr;

    std::shared_ptr<ITexture> m_progress   = nullptr;
    std::shared_ptr<ITexture> m_background = nullptr;

    std::shared_ptr<IBuffer>      m_vertex_buffer_back     = nullptr;
    std::shared_ptr<IBuffer>      m_vertex_buffer_progress = nullptr;
    std::shared_ptr<IIndexBuffer> m_index_buffer           = nullptr;

    UBO m_progress_ubo;
    UBO m_background_ubo;

    ProgressImageInfo m_progress_info;

    float m_background_alpha = 0.0F;
    float m_fade_speed       = 1.0F;

    uint32_t m_current_frame = 0;
};

}  // namespace storm
