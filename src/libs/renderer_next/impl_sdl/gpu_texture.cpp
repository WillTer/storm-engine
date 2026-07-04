#include "gpu_texture.h"

#include <cassert>
#include <format>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/texture_asset.h>
#include <spdlog/spdlog.h>

#include "gpu_render_pass.h"
#include "gpu_sampler.h"

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

GPUTexture::GPUTexture(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<GPUSampler> const& sampler, TextureAsset const& asset)
    : GPUTexture(
          device,
          sampler,
          asset.header.width,
          asset.header.height,
          asset.header.mip_levels,
          convert_tx_format(asset.header.format),
          SDL_GPU_TEXTUREUSAGE_SAMPLER)
{
    auto texture_transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    texture_transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    texture_transfer_buffer_create_info.size  = static_cast<uint32_t>(asset.data.size());

    m_transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &texture_transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!m_transfer_buffer) {
        throw std::runtime_error(std::format("Failed to create transfer buffer for texture: {}", SDL_GetError()));
    }

    char* const transfer_data = static_cast<char*>(SDL_MapGPUTransferBuffer(device.get(), m_transfer_buffer.get(), false));
    std::memcpy(transfer_data, asset.data.data(), asset.data.size());
    SDL_UnmapGPUTransferBuffer(device.get(), m_transfer_buffer.get());
}

GPUTexture::GPUTexture(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<GPUSampler> const&    sampler,
    uint32_t const                        width,
    uint32_t const                        height,
    uint32_t const                        mip_levels,
    int32_t const                         format,
    uint32_t const                        usage)
    : m_sampler(sampler)
    , m_width(width)
    , m_height(height)
{
    // Create default one, if not presented
    if (!m_sampler) {
        m_sampler = std::make_shared<GPUSampler>(
            device,
            GPUSampler::Info {
                .min_filter     = GPUSampler::Filter::Linear,
                .mag_filter     = GPUSampler::Filter::Linear,
                .mipmap_filter  = GPUSampler::Filter::Linear,
                .address_mode   = GPUSampler::AddressMode::Repeat,
                .max_anisotropy = std::nullopt,
            });
    }

    auto texture_create_info                 = SDL_GPUTextureCreateInfo {};
    texture_create_info.type                 = SDL_GPU_TEXTURETYPE_2D;
    texture_create_info.format               = static_cast<SDL_GPUTextureFormat>(format);
    texture_create_info.usage                = usage;
    texture_create_info.width                = m_width;
    texture_create_info.height               = m_height;
    texture_create_info.layer_count_or_depth = 1;
    texture_create_info.num_levels           = mip_levels;

    m_texture = std::shared_ptr<SDL_GPUTexture>(
        SDL_CreateGPUTexture(device.get(), &texture_create_info), [device](SDL_GPUTexture* p) { SDL_ReleaseGPUTexture(device.get(), p); });
    if (!m_texture) {
        throw std::runtime_error(std::format("Failed to create texture: {}", SDL_GetError()));
    }
}

GPUTexture::~GPUTexture() = default;

void GPUTexture::set_as_target(ColorTargetInfo& target_info)
{
    target_info.target_texture = m_texture.get();
}

void GPUTexture::bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const
{
    assert(render_pass);

    auto const texture_binding = SDL_GPUTextureSamplerBinding {
        .texture = m_texture.get(),
        .sampler = *m_sampler,
    };

    SDL_BindGPUFragmentSamplers(render_pass.get(), 0, &texture_binding, 1);
}

std::pair<uint32_t, uint32_t> GPUTexture::get_dimensions() const
{
    return std::make_pair(m_width, m_height);
}

void GPUTexture::upload(std::shared_ptr<SDL_GPUCopyPass> const& copy_pass)
{
    if (!m_transfer_buffer) {
        return;
    }

    auto const tex_transfer_location = SDL_GPUTextureTransferInfo {
        .transfer_buffer = m_transfer_buffer.get(),
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
        .w         = m_width,
        .h         = m_height,
        .d         = 1,
    };

    SDL_UploadToGPUTexture(copy_pass.get(), &tex_transfer_location, &tex_buffer_region, false);

    m_transfer_buffer.reset();
}
