#include "texture_atlas_named.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_copy_pass.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

#include "image_2d.h"

using namespace storm;

TextureAtlasNamed::TextureAtlasNamed(std::filesystem::path const& texture)
{
    auto const& renderer = core->get<RendererService>();
    m_texture            = renderer->create_texture(texture.string());

    std::tie(m_width, m_height) = m_texture->get_dimensions();
}

TextureAtlasNamed::~TextureAtlasNamed() = default;

void TextureAtlasNamed::update(GPUCopyPass const& /*copy_pass*/, uint64_t /*delta_time*/) {}

void TextureAtlasNamed::add_picture(std::string const& name, storm::FRect const& texture_pos_rect)
{
    storm::FRect const texture_uv_rect = {
        .left   = static_cast<float>(texture_pos_rect.left) / m_width,
        .top    = static_cast<float>(texture_pos_rect.top) / m_height,
        .right  = static_cast<float>(texture_pos_rect.right) / m_width,
        .bottom = static_cast<float>(texture_pos_rect.bottom) / m_height,
    };

    m_pictures.emplace(name, texture_uv_rect);
}

auto TextureAtlasNamed::get_tex_coords(std::string const& name) const -> storm::FRect
{
    if (!m_pictures.contains(name)) { return {}; }

    return m_pictures.at(name);
}

auto TextureAtlasNamed::get_texture() const -> std::shared_ptr<GPUTexture>
{
    return m_texture;
}
