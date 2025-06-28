#include "gpu_index_buffer.h"

#include <libs/core/core.h>
#include <spdlog/spdlog.h>

#include "graphics_pipeline.h"
#include "renderer_sdl.h"

using namespace storm;

GPUIndexBuffer::GPUIndexBuffer(
    RendererService& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, void const* data, uint32_t data_size, uint32_t index_size)
    : m_renderer(renderer)
    , m_buffer(std::make_unique<GPUBuffer>(renderer))
    , m_index_count(data_size / index_size)
    , m_element_size(index_size == 2 ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT)
{
    auto const device = m_renderer.get_device();

    auto buffer_create_info  = SDL_GPUBufferCreateInfo {};
    buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    buffer_create_info.size  = data_size;

    auto const buffer = std::shared_ptr<SDL_GPUBuffer>(
        SDL_CreateGPUBuffer(device.get(), &buffer_create_info), [device](SDL_GPUBuffer* p) { SDL_ReleaseGPUBuffer(device.get(), p); });
    if (!buffer) { throw std::runtime_error(std::format("Failed to create index buffer: {}", SDL_GetError())); }

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
        .buffer = buffer.get(),
        .offset = 0,
        .size   = data_size,
    };
    SDL_UploadToGPUBuffer(copy_pass.get(), &transfer_location, &buffer_region, false);

    m_buffer->set_gpu_buffer(buffer);
}

GPUIndexBuffer::~GPUIndexBuffer() = default;

void GPUIndexBuffer::bind_to_render_pass() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    assert(render_pass);

    auto index_binding   = SDL_GPUBufferBinding {};
    index_binding.buffer = m_buffer->get_gpu_buffer().get();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(render_pass.get(), &index_binding, m_element_size);
}

void GPUIndexBuffer::draw_indexed() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    assert(render_pass);

    SDL_DrawGPUIndexedPrimitives(render_pass.get(), m_index_count, 1, 0, 0, 0);
}

void GPUIndexBuffer::update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t const stride)
{
    m_buffer->update_data(update_info, data, stride);
}
