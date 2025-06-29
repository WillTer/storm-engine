#include "gpu_buffer.h"

#include <numeric>

#include <SDL3/SDL_gpu.h>
#include <spdlog/spdlog.h>

using namespace storm;

GPUBuffer::GPUBuffer() {}
GPUBuffer::~GPUBuffer() = default;

void GPUBuffer::update(
    std::shared_ptr<SDL_GPUDevice> const&   device,
    std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
    std::vector<BufferUpdateInfo> const&    update_info,
    void const*                             data,
    size_t const                            stride) const
{
    auto const transfer_buffer_size = std::accumulate(
        update_info.begin(), update_info.end(), uint32_t {0}, [](uint32_t acc, BufferUpdateInfo const& info) { return acc + info.size; });

    auto transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer_create_info.size  = transfer_buffer_size;

    auto const transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!transfer_buffer) {
        spdlog::error("Failed to create vertex transfer buffer: {}", SDL_GetError());
        return;
    }

    void* const transfer_data = SDL_MapGPUTransferBuffer(device.get(), transfer_buffer.get(), false);
    std::memcpy(transfer_data, data, transfer_buffer_size);
    SDL_UnmapGPUTransferBuffer(device.get(), transfer_buffer.get());

    uint32_t location_offset = 0;
    uint32_t buffer_offset   = 0;
    for (auto const& info: update_info) {
        auto const transfer_location = SDL_GPUTransferBufferLocation {
            .transfer_buffer = transfer_buffer.get(),
            .offset          = location_offset,
        };

        auto const buffer_region = SDL_GPUBufferRegion {
            .buffer = m_buffer.get(),
            .offset = buffer_offset + info.offset,
            .size   = info.size,
        };

        SDL_UploadToGPUBuffer(copy_pass.get(), &transfer_location, &buffer_region, false);

        location_offset += info.size;
        buffer_offset += static_cast<uint32_t>(stride);
    }
}

void GPUBuffer::upload(
    std::shared_ptr<SDL_GPUDevice> const&   device,
    std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
    void const*                             data,
    uint32_t                                data_size) const
{
    auto transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer_create_info.size  = data_size;

    auto const transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!transfer_buffer) { throw std::runtime_error(std::format("Failed to create vertex transfer buffer: {}", SDL_GetError())); }

    void* const transfer_data = SDL_MapGPUTransferBuffer(device.get(), transfer_buffer.get(), false);
    std::memcpy(transfer_data, data, data_size);
    SDL_UnmapGPUTransferBuffer(device.get(), transfer_buffer.get());

    auto const transfer_location = SDL_GPUTransferBufferLocation {
        .transfer_buffer = transfer_buffer.get(),
        .offset          = 0,
    };

    auto const buffer_region = SDL_GPUBufferRegion {
        .buffer = m_buffer.get(),
        .offset = 0,
        .size   = data_size,
    };

    SDL_UploadToGPUBuffer(copy_pass.get(), &transfer_location, &buffer_region, false);
}
