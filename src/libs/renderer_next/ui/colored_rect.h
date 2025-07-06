#pragma once

#include <memory>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/types.h>
#include <shaders/ui/colored_rect.h>
#include <shaders/ui/ubo_types.h>

#include "image_2d_base.h"

namespace storm
{

class GraphicsPipeline;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUCopyPass;
class GPURenderPass;

class ColoredRect final: public Image2DBase
{
public:
    ColoredRect(storm::Color const& color);
    ~ColoredRect() override;

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void draw(GPURenderPass const& render_pass) const override;

    void set_color(storm::Color const& color);

private:
    storm::Color m_color;
    bool         m_is_color_dirty;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
