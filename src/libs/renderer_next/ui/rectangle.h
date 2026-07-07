#pragma once

#include <memory>
#include <vector>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/i_drawable.h>
#include <libs/renderer_next/types.h>
#include <shaders/ubo_types.h>

#include "base.h"

namespace storm::renderer::ui
{

class Rectangle: public Base
{
public:
    Rectangle(storm::Color const& color, std::optional<std::string> const& technique = std::nullopt);
    Rectangle(uint32_t const& color, std::optional<std::string> const& technique = std::nullopt);
    Rectangle(std::array<storm::Color, 4> const& colors, std::optional<std::string> const& technique = std::nullopt);
    Rectangle(std::array<uint32_t, 4> const& colors, std::optional<std::string> const& technique = std::nullopt);

    virtual ~Rectangle();

    virtual void set_vertices_colors(std::array<storm::Color, 4> const& colors);
    virtual void set_vertices_colors(std::array<uint32_t, 4> const& colors);
    virtual void set_vertices_color(storm::Color const& color);
    virtual void set_vertices_color(uint32_t const& color);

    // Base
    void create_default_pipeline(std::string const& fragment_shader = {}) override;
    void set_technique(std::string const& technique, std::string const& vertex_shader = {}) override;

    // IDrawable
    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void draw(GPURenderPass const& render_pass) const override;

private:
    void initialize(std::optional<std::string> const& technique);

    bool m_need_update = false;

    std::vector<float4> m_colors;
};

}  // namespace storm::renderer::ui
