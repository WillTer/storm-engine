#include "gpu_vertex_buffer.h"

#include <libs/core/core.h>
#include <spdlog/spdlog.h>

#include "renderer_sdl.h"

using namespace storm;

GPUVertexBuffer::GPUVertexBuffer(
    RendererService& renderer, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass, void const* vertex_data, uint32_t vertex_data_size)
    : m_renderer(renderer)
    , m_buffer(std::make_unique<GPUBuffer>(renderer))
{
    auto const device = m_renderer.get_device();

    auto vertex_buffer_create_info  = SDL_GPUBufferCreateInfo {};
    vertex_buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer_create_info.size  = vertex_data_size;

    auto const buffer =
        std::shared_ptr<SDL_GPUBuffer>(SDL_CreateGPUBuffer(device.get(), &vertex_buffer_create_info), [device](SDL_GPUBuffer* p) {
            SDL_ReleaseGPUBuffer(device.get(), p);
        });
    if (!buffer) { throw std::runtime_error(std::format("Failed to create vertex buffer: {}", SDL_GetError())); }

    auto vertex_transfer_buffer_create_info  = SDL_GPUTransferBufferCreateInfo {};
    vertex_transfer_buffer_create_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    vertex_transfer_buffer_create_info.size  = vertex_data_size;

    auto const vertex_transfer_buffer = std::shared_ptr<SDL_GPUTransferBuffer>(
        SDL_CreateGPUTransferBuffer(device.get(), &vertex_transfer_buffer_create_info),
        [device](SDL_GPUTransferBuffer* p) { SDL_ReleaseGPUTransferBuffer(device.get(), p); });
    if (!vertex_transfer_buffer) { throw std::runtime_error(std::format("Failed to create vertex transfer buffer: {}", SDL_GetError())); }

    char* const vertex_transfer_data = static_cast<char*>(SDL_MapGPUTransferBuffer(device.get(), vertex_transfer_buffer.get(), false));
    std::memcpy(vertex_transfer_data, vertex_data, vertex_data_size);
    SDL_UnmapGPUTransferBuffer(device.get(), vertex_transfer_buffer.get());

    auto const vertex_transfer_location = SDL_GPUTransferBufferLocation {
        .transfer_buffer = vertex_transfer_buffer.get(),
        .offset          = 0,
    };
    auto const vertex_buffer_region = SDL_GPUBufferRegion {
        .buffer = buffer.get(),
        .offset = 0,
        .size   = vertex_data_size,
    };
    SDL_UploadToGPUBuffer(copy_pass.get(), &vertex_transfer_location, &vertex_buffer_region, false);

    m_buffer->set_gpu_buffer(buffer);
}

GPUVertexBuffer::~GPUVertexBuffer() = default;

void GPUVertexBuffer::bind_to_render_pass() const
{
    auto const& render_pass = m_renderer.get_current_render_pass();
    assert(render_pass);

    auto vertex_binding   = SDL_GPUBufferBinding {};
    vertex_binding.buffer = m_buffer->get_gpu_buffer().get();
    vertex_binding.offset = 0;
    SDL_BindGPUVertexBuffers(render_pass.get(), 0, &vertex_binding, 1);
}

void GPUVertexBuffer::update_data(std::vector<BufferUpdateInfo> const& update_info, void const* data, uint32_t const stride)
{
    m_buffer->update_data(update_info, data, stride);
}
