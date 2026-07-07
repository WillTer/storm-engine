#pragma once

#include <filesystem>
#include <memory>

#include <libs/renderer_next/hlslpp.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/types.h>
#include <shaders/ubo_types.h>

#include "base.h"

namespace storm::renderer::ui
{

class Image2D: public Base
{
public:
    Image2D(std::filesystem::path const& texture, storm::FRect const& uv = {}, std::optional<std::string> const& technique = std::nullopt);
    Image2D(
        std::shared_ptr<GPUTexture> const& external_texture,
        storm::FRect const&                uv        = {},
        std::optional<std::string> const&  technique = std::nullopt);

    ~Image2D() override;

    virtual auto get_uv() const -> std::vector<float2> const&;

    virtual void set_uv_rect(storm::FRect const& uv);
    virtual void set_uv(std::array<float2, 4> const& uv);
    virtual void set_ubo_color(storm::Color const& color);

    // Base
    void create_default_pipeline(std::string const& fragment_shader = {}) override;
    void set_technique(std::string const& technique, std::string const& vertex_shader = {}) override;

    // IDrawable
    void update(GPUCopyPass const& copy_pass, uint64_t delta_time) override;
    void draw(GPURenderPass const& render_pass) const override;

protected:
    std::shared_ptr<GPUTexture> m_texture;
    std::vector<float2>         m_texture_uv;

private:
    void initialize(storm::FRect const& uv, std::optional<std::string> const& technique);
    void update_rect();

    bool m_need_update = false;
};

}  // namespace storm::renderer::ui
