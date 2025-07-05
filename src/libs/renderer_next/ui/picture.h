#pragma once

#include <filesystem>
#include <memory>

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
class GPUCopyPass;
class GPURenderPass;

class Picture: public Image2DBase
{
public:
    Picture(GPUCopyPass const& copy_pass, std::filesystem::path const& texture, storm::FRect const& texture_rect = default_texture_rect());
    Picture(
        GPUCopyPass const&                 copy_pass,
        std::shared_ptr<GPUTexture> const& external_texture,
        storm::FRect const&                texture_rect = default_texture_rect());

    ~Picture() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_diffuse_color(storm::Color const& color);

private:
    void initialize(GPUCopyPass const& copy_pass, storm::FRect const& texture_rect);

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

    std::unique_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture>      m_texture;
    std::unique_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::unique_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
