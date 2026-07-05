#include "gpu_sampler.h"

#include <cassert>
#include <format>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <libs/asset_server/asset_server.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/config/i_config_loader.h>
#include <libs/filesystem/default_paths.h>
#include <spdlog/spdlog.h>

using namespace storm;

namespace
{

SDL_GPUFilter convert_gpu_filter(technique::Filter filter)
{
    switch (filter) {
    case technique::Filter::Nearest: return SDL_GPU_FILTER_NEAREST;
    case technique::Filter::Linear: return SDL_GPU_FILTER_LINEAR;
    }

    return {};
}

SDL_GPUSamplerMipmapMode convert_mipmap_filter(technique::Filter filter)
{
    switch (filter) {
    case technique::Filter::Nearest: return SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    case technique::Filter::Linear: return SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    }

    return {};
}

SDL_GPUSamplerAddressMode convert_address_mode(technique::AddressMode address_mode)
{
    switch (address_mode) {
    case technique::AddressMode::Repeat: return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    case technique::AddressMode::Mirror: return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    case technique::AddressMode::Clamp: return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    }

    return {};
}

}  // namespace

GPUSampler::GPUSampler(std::shared_ptr<SDL_GPUDevice> const& device, SamplerInfo const& info)
{
    initialize(device, info);
}

GPUSampler::GPUSampler(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<AssetServer> const&   asset_server,
    std::shared_ptr<IConfigLoader> const& config_loader,
    std::string const&                    technique)
{
    auto const& technique_info =
        technique::info(*config_loader, asset_server->get_asset_dir<ShaderAsset>() / fs::TECHNIQUES_FILE, technique);

    initialize(device, technique_info.sampler);
}

GPUSampler::~GPUSampler() = default;

GPUSampler::operator SDL_GPUSampler*() const
{
    return m_sampler.get();
}

void GPUSampler::initialize(std::shared_ptr<SDL_GPUDevice> const& device, SamplerInfo const& info)
{
    auto sampler_create_info              = SDL_GPUSamplerCreateInfo {};
    sampler_create_info.min_filter        = convert_gpu_filter(info.min_filter);
    sampler_create_info.mag_filter        = convert_gpu_filter(info.mag_filter);
    sampler_create_info.mipmap_mode       = convert_mipmap_filter(info.mipmap_mode);
    sampler_create_info.enable_anisotropy = true;
    sampler_create_info.max_anisotropy    = 16.0F;

    sampler_create_info.address_mode_u = convert_address_mode(info.address_mode_u);
    sampler_create_info.address_mode_v = convert_address_mode(info.address_mode_v);
    sampler_create_info.address_mode_w = convert_address_mode(info.address_mode_w);

    m_sampler = std::shared_ptr<SDL_GPUSampler>(
        SDL_CreateGPUSampler(device.get(), &sampler_create_info), [device](SDL_GPUSampler* p) { SDL_ReleaseGPUSampler(device.get(), p); });

    if (!m_sampler) {
        throw std::runtime_error(std::format("Failed to create GPU Sampler: {}", SDL_GetError()));
    }
}
