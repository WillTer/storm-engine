#include "texture_sdl.h"

#include <cassert>
#include <format>
#include <stdexcept>

#include <spdlog/spdlog.h>

#include "renderer_sdl.h"

using namespace storm;

namespace
{

SDL_GPUTextureFormat convert_tx_format(TxFormat const format)
{
    switch (format) {
    case TxFormat::A8R8G8B8: return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
    case TxFormat::X8R8G8B8: return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
    case TxFormat::R5G6B5: return SDL_GPU_TEXTUREFORMAT_B5G6R5_UNORM;
    case TxFormat::A1R5G5B5: return SDL_GPU_TEXTUREFORMAT_B5G5R5A1_UNORM;
    case TxFormat::A4R4G4B4: return SDL_GPU_TEXTUREFORMAT_B4G4R4A4_UNORM;
    case TxFormat::L8: return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
    case TxFormat::V8U8: return SDL_GPU_TEXTUREFORMAT_R8G8_SNORM;

    case TxFormat::DXT1: return SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM;
    case TxFormat::DXT2: return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
    case TxFormat::DXT3: return SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM;
    case TxFormat::DXT4: return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
    case TxFormat::DXT5: return SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM;
    default: break;
    }

    return SDL_GPU_TEXTUREFORMAT_INVALID;
}

}  // namespace

TextureSDL::TextureSDL(RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, TextureAsset const& asset)
    : m_renderer(renderer)
{
    auto const& device = renderer.get_device();

    auto sampler_create_info              = SDL_GPUSamplerCreateInfo {};
    sampler_create_info.min_filter        = SDL_GPU_FILTER_LINEAR;
    sampler_create_info.mag_filter        = SDL_GPU_FILTER_LINEAR;
    sampler_create_info.mipmap_mode       = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    sampler_create_info.address_mode_u    = SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    sampler_create_info.address_mode_v    = SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    sampler_create_info.address_mode_w    = SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    sampler_create_info.enable_anisotropy = true;   // FIXME: configurable
    sampler_create_info.max_anisotropy    = 16.0F;  // FIXME: configurable

    m_sampler = std::shared_ptr<SDL_GPUSampler>(
        SDL_CreateGPUSampler(device.get(), &sampler_create_info), [device](SDL_GPUSampler* p) { SDL_ReleaseGPUSampler(device.get(), p); });

    if (!m_sampler) { throw std::runtime_error(std::format("Failed to create GPU Sampler: {}", SDL_GetError())); }

    // TODO: support cubemaps (array too?)
    auto texture_create_info                 = SDL_GPUTextureCreateInfo {};
    texture_create_info.type                 = SDL_GPU_TEXTURETYPE_2D;
    texture_create_info.format               = convert_tx_format(asset.header.format);
    texture_create_info.usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texture_create_info.width                = asset.header.width;
    texture_create_info.height               = asset.header.height;
    texture_create_info.layer_count_or_depth = 1;
    texture_create_info.num_levels           = asset.header.mip_levels;

    m_texture = std::shared_ptr<SDL_GPUTexture>(
        SDL_CreateGPUTexture(device.get(), &texture_create_info), [device](SDL_GPUTexture* p) { SDL_ReleaseGPUTexture(device.get(), p); });
    if (!m_texture) { throw std::runtime_error(std::format("Failed to create texture: {}", SDL_GetError())); }

    auto texture_transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    texture_transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    texture_transfer_buffer_create_info.size  = static_cast<Uint32>(asset.data.size());

    auto const texture_transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &texture_transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!texture_transfer_buffer) {
        throw std::runtime_error(std::format("Failed to create transfer buffer for texture: {}", SDL_GetError()));
    }

    char* const transfer_data = static_cast<char*>(SDL_MapGPUTransferBuffer(device.get(), texture_transfer_buffer.get(), false));
    std::memcpy(transfer_data, asset.data.data(), asset.data.size());
    SDL_UnmapGPUTransferBuffer(device.get(), texture_transfer_buffer.get());

    auto const tex_transfer_location = SDL_GPUTextureTransferInfo {
        .transfer_buffer = texture_transfer_buffer.get(),
        .offset          = 0,
        .pixels_per_row  = 0,
        .rows_per_layer  = 0,
    };
    auto const tex_buffer_region = SDL_GPUTextureRegion {
        .texture   = m_texture.get(),
        .mip_level = 0,
        .layer     = 0,
        .x         = 0,
        .y         = 0,
        .z         = 0,
        .w         = asset.header.width,
        .h         = asset.header.height,
        .d         = 1,
    };
    SDL_UploadToGPUTexture(copy_pass.get(), &tex_transfer_location, &tex_buffer_region, false);

    m_width  = asset.header.width;
    m_height = asset.header.height;
}

TextureSDL::~TextureSDL() = default;

void TextureSDL::bind_to_render_pass() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    assert(render_pass);

    auto const texture_binding = SDL_GPUTextureSamplerBinding {
        .texture = m_texture.get(),
        .sampler = m_sampler.get(),
    };

    SDL_BindGPUFragmentSamplers(render_pass.get(), 0, &texture_binding, 1);
}

std::pair<uint32_t, uint32_t> TextureSDL::get_dimensions() const
{
    return std::make_pair(m_width, m_height);
}
