#pragma once

#include <memory>

#include <libs/config/font.h>
#include <shaders/ubo_types.h>

namespace storm
{

class GPUCommandBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUVertexBuffer;

class GraphicsPipeline;

class Font final
{
public:
    enum class Alignment { Left, Center, Right };

    Font(std::string const& font_name);
    ~Font();

    float get_string_width(std::string const& text, std::optional<float> scale_override) const;

    auto print(
        GPUCommandBuffer const& cmd_buffer,
        storm::Color            fg,
        storm::Color            bg,
        Alignment               align,
        bool                    draw_shadow,
        float                   scale,
        float                   width,
        float                   height,
        std::string const&      text) -> std::shared_ptr<GPUTexture>;

private:
    FontInfo m_info;

    shaders::FontUBOVertex m_vertex_ubo;
    shaders::UBOFragment   m_fragment_ubo;

    std::shared_ptr<GraphicsPipeline> m_pipeline;

    std::shared_ptr<GPUTexture> m_texture;

    std::shared_ptr<GPUVertexBuffer> m_vertex_buffer;
    std::shared_ptr<GPUIndexBuffer>  m_index_buffer;
};

}  // namespace storm
