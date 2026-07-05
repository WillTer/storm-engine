#pragma once

#include <memory>

#include <libs/renderer_next/types.h>
#include <shaders/ubo_types.h>
#include <shaders/ui/image_2d.h>

#include "image_2d_base.h"

namespace storm
{

class GraphicsPipeline;
class GPUCopyPass;
class GPUIndexBuffer;
class GPURenderPass;
class GPUTexture;
class GPUVertexBuffer;
class Image2D;

class Button final: public Image2DBase
{
public:
    Button(
        std::shared_ptr<GPUTexture> const& texture,
        storm::FRect const&                tex_rect_left,
        storm::FRect const&                tex_rect_middle,
        storm::FRect const&                tex_rect_right,
        storm::FRect const&                button_rect,
        std::optional<std::string> const&  technique = std::nullopt);
    ~Button() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void draw(GPURenderPass const& render_pass) const override;

    void set_diffuse_color(storm::Color const& color);

    auto get_middle_rect() const -> storm::FRect;

private:
    storm::FRect m_middle_rect;

    shaders::UBOFragment m_fragment_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_left;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_middle;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_right;

    std::shared_ptr<GPUTexture>     m_texture;
    std::shared_ptr<GPUIndexBuffer> m_index_buffer;
};

}  // namespace storm
