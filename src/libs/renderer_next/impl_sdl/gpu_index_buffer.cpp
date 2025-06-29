#include "gpu_index_buffer.h"

#include <SDL3/SDL_gpu.h>
#include <libs/core/core.h>
#include <spdlog/spdlog.h>

using namespace storm;

GPUIndexBuffer::GPUIndexBuffer(std::shared_ptr<SDL_GPUDevice> const& device, uint32_t const index_count) : m_index_count(index_count)
{
    auto buffer_create_info  = SDL_GPUBufferCreateInfo {};
    buffer_create_info.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    buffer_create_info.size  = index_count * sizeof(uint32_t);  // Only uint32_t supported for index

    m_buffer = std::shared_ptr<SDL_GPUBuffer>(
        SDL_CreateGPUBuffer(device.get(), &buffer_create_info), [device](SDL_GPUBuffer* p) { SDL_ReleaseGPUBuffer(device.get(), p); });
    if (!m_buffer) { throw std::runtime_error(std::format("Failed to create index buffer: {}", SDL_GetError())); }
}

GPUIndexBuffer::~GPUIndexBuffer() = default;

void GPUIndexBuffer::bind_to_render_pass(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const
{
    assert(render_pass);

    auto index_binding   = SDL_GPUBufferBinding {};
    index_binding.buffer = m_buffer.get();
    index_binding.offset = 0;
    SDL_BindGPUIndexBuffer(render_pass.get(), &index_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
}

void GPUIndexBuffer::draw(std::shared_ptr<SDL_GPURenderPass> const& render_pass) const
{
    assert(render_pass);
    SDL_DrawGPUIndexedPrimitives(render_pass.get(), m_index_count, 1, 0, 0, 0);
}
