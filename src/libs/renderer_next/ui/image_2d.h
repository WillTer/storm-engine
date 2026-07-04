#pragma once

#include <filesystem>
#include <memory>

#include <entt/core/fwd.hpp>
#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/types.h>
#include <shaders/ui/image_2d.h>
#include <shaders/ui/ubo_types.h>

#include "image_2d_base.h"

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCopyPass;
class GPURenderPass;

class Image2D final: public Image2DBase
{
public:
    Image2D(std::filesystem::path const& texture, std::shared_ptr<GPUSampler> const& sampler = nullptr);
    Image2D(std::shared_ptr<GPUTexture> const& external_texture);

    ~Image2D() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void draw(GPURenderPass const& render_pass) const override;

    void set_pipeline(entt::hashed_string const& name);
    void set_diffuse_color(storm::Color const& color);
    void set_uv(storm::FRect const& texture_uv);
    void set_uv_full(std::array<float2, 4> const& texture_uv);

private:
    void initialize();

    shaders::UBOFragment m_fragment_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture>      m_texture;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer;

    std::vector<float2> m_texture_uv;
    bool                m_is_dirty {false};
};

}  // namespace storm
