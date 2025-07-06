#pragma once

#include <filesystem>
#include <memory>

#include <libs/renderer_next/hlslpp.h>
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
    Image2D(std::filesystem::path const& texture, storm::FRect const& texture_rect = default_texture_rect());
    Image2D(std::shared_ptr<GPUTexture> const& external_texture, storm::FRect const& texture_rect = default_texture_rect());

    ~Image2D() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void draw(GPURenderPass const& render_pass) const override;

    void set_diffuse_color(storm::Color const& color);

private:
    void initialize(storm::FRect const& texture_rect);

    static constexpr storm::FRect default_texture_rect()
    {
        return storm::FRect {
            .left   = 0.0F,
            .top    = 0.0F,
            .right  = 1.0F,
            .bottom = 1.0F,
        };
    }

    shaders::UBOFragment m_fragment_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture>      m_texture;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
