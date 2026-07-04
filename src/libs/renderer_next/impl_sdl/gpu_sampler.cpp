#include "gpu_sampler.h"

#include <cassert>
#include <format>
#include <stdexcept>

#include <SDL3/SDL_gpu.h>
#include <spdlog/spdlog.h>

using namespace storm;

namespace
{

SDL_GPUFilter convert_gpu_filter(GPUSampler::Filter filter)
{
    switch (filter) {
    case GPUSampler::Filter::Nearest: return SDL_GPU_FILTER_NEAREST;
    case GPUSampler::Filter::Linear: return SDL_GPU_FILTER_LINEAR;
    }

    return {};
}

SDL_GPUSamplerMipmapMode convert_mipmap_filter(GPUSampler::Filter filter)
{
    switch (filter) {
    case GPUSampler::Filter::Nearest: return SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    case GPUSampler::Filter::Linear: return SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    }

    return {};
}

SDL_GPUSamplerAddressMode convert_address_mode(GPUSampler::AddressMode address_mode)
{
    switch (address_mode) {
    case GPUSampler::AddressMode::Repeat: return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
    case GPUSampler::AddressMode::MirroredRepeat: return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
    case GPUSampler::AddressMode::Clamp: return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    }

    return {};
}

}  // namespace

GPUSampler::GPUSampler(std::shared_ptr<SDL_GPUDevice> const& device, Info info)
{
    auto sampler_create_info              = SDL_GPUSamplerCreateInfo {};
    sampler_create_info.min_filter        = convert_gpu_filter(info.min_filter);
    sampler_create_info.mag_filter        = convert_gpu_filter(info.mag_filter);
    sampler_create_info.mipmap_mode       = convert_mipmap_filter(info.mipmap_filter);
    sampler_create_info.enable_anisotropy = info.max_anisotropy.has_value();
    sampler_create_info.max_anisotropy    = info.max_anisotropy.value_or(0.0F);

    auto const sampler_address_mode = convert_address_mode(info.address_mode);

    sampler_create_info.address_mode_u = sampler_address_mode;
    sampler_create_info.address_mode_v = sampler_address_mode;
    sampler_create_info.address_mode_w = sampler_address_mode;

    m_sampler = std::shared_ptr<SDL_GPUSampler>(
        SDL_CreateGPUSampler(device.get(), &sampler_create_info), [device](SDL_GPUSampler* p) { SDL_ReleaseGPUSampler(device.get(), p); });

    if (!m_sampler) {
        throw std::runtime_error(std::format("Failed to create GPU Sampler: {}", SDL_GetError()));
    }
}

GPUSampler::~GPUSampler() = default;

GPUSampler::operator SDL_GPUSampler*() const
{
    return m_sampler.get();
}
