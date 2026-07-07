#pragma once

#include <memory>

#include <libs/renderer_next/types.h>
#include <shaders/ubo_types.h>

#include "image_2d.h"

namespace storm::renderer::ui
{

class Button final: public Image2D
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

    auto get_middle_rect() const -> storm::FRect const&;

private:
    storm::FRect m_middle_rect;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_left;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_middle;
    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer_right;
};

}  // namespace storm::renderer::ui
