#include "index_buffer_sdl.h"

#include <filesystem>

#include <libs/core/core.h>
#include <spdlog/spdlog.h>

#include "pipeline_sdl.h"
#include "renderer_sdl.h"

using namespace storm;

IndexBufferSDL::IndexBufferSDL(
    RendererSDL& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, std::vector<uint16_t> const& indices)
    : m_renderer(renderer)
    , m_buffer(std::make_unique<BufferSDL>(renderer))
    , m_index_count(static_cast<uint32_t>(indices.size()))
{
    auto const device = m_renderer.get_device();

    uint32_t const index_data_size    = sizeof(indices[0]) * m_index_count;
    auto           buffer_create_info = SDL_GPUBufferCreateInfo {};
    buffer_create_info.usage          = SDL_GPU_BUFFERUSAGE_INDEX;
    buffer_create_info.size           = index_data_size;

    auto const buffer = std::shared_ptr<SDL_GPUBuffer>(
        SDL_CreateGPUBuffer(device.get(), &buffer_create_info), [device](SDL_GPUBuffer* p) { SDL_ReleaseGPUBuffer(device.get(), p); });
    if (!buffer) { throw std::runtime_error(std::format("Failed to create index buffer: {}", SDL_GetError())); }

    auto transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer_create_info.size  = index_data_size;

    auto const transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!transfer_buffer) { throw std::runtime_error(std::format("Failed to create vertex transfer buffer: {}", SDL_GetError())); }

    void* const transfer_data = SDL_MapGPUTransferBuffer(device.get(), transfer_buffer.get(), false);
    std::memcpy(transfer_data, indices.data(), index_data_size);
    SDL_UnmapGPUTransferBuffer(device.get(), transfer_buffer.get());

    auto const transfer_location = SDL_GPUTransferBufferLocation {
        .transfer_buffer = transfer_buffer.get(),
        .offset          = 0,
    };
    auto const buffer_region = SDL_GPUBufferRegion {
        .buffer = buffer.get(),
        .offset = 0,
        .size   = index_data_size,
    };
    SDL_UploadToGPUBuffer(copy_pass.get(), &transfer_location, &buffer_region, false);

    m_buffer->set_gpu_buffer(buffer);
}

IndexBufferSDL::~IndexBufferSDL() = default;

void IndexBufferSDL::bind_to_render_pass() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    assert(render_pass);

    auto index_binding   = SDL_GPUBufferBinding {};
    index_binding.buffer = m_buffer->get_gpu_buffer().get();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(render_pass.get(), &index_binding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
}

void IndexBufferSDL::draw_indexed() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    assert(render_pass);

    SDL_DrawGPUIndexedPrimitives(render_pass.get(), m_index_count, 1, 0, 0, 0);
}

void IndexBufferSDL::update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t const stride)
{
    m_buffer->update_data(update_info, data, stride);
}
