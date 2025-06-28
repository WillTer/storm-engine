#include "gpu_buffer.h"

#include <numeric>

#include <spdlog/spdlog.h>

#include "renderer_sdl.h"

using namespace storm;

GPUBuffer::GPUBuffer(RendererService& renderer) : m_renderer(renderer) {}
GPUBuffer::~GPUBuffer() = default;

void GPUBuffer::bind_to_render_pass() const {}

void GPUBuffer::update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t stride)
{
    auto const& device = m_renderer.get_device();

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

    auto const cmd_buffer = std::shared_ptr<SDL_GPUCommandBuffer>(SDL_AcquireGPUCommandBuffer(device.get()), &SDL_SubmitGPUCommandBuffer);
    auto const copy_pass  = std::shared_ptr<SDL_GPUCopyPass>(SDL_BeginGPUCopyPass(cmd_buffer.get()), &SDL_EndGPUCopyPass);
    uint32_t   location_offset = 0;
    uint32_t   buffer_offset   = 0;
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
        buffer_offset += stride;
    }
}

void GPUBuffer::set_gpu_buffer(std::shared_ptr<SDL_GPUBuffer> const& buffer)
{
    assert(buffer);
    m_buffer = buffer;
}

std::shared_ptr<SDL_GPUBuffer> GPUBuffer::get_gpu_buffer() const
{
    return m_buffer;
}
