#include "gpu_copy_pass.h"

#include <SDL3/SDL_gpu.h>
#include <spdlog/spdlog.h>

using namespace storm;

GPUCopyPass::GPUCopyPass(std::shared_ptr<SDL_GPUDevice> const& device, std::shared_ptr<SDL_GPUCommandBuffer> const& cmd_buffer)
    : m_device(device)
{
    m_pass = std::shared_ptr<SDL_GPUCopyPass>(SDL_BeginGPUCopyPass(cmd_buffer.get()), &SDL_EndGPUCopyPass);
    if (!m_pass) { spdlog::error("Begin GPU copy pass failed: {}", SDL_GetError()); }
}

GPUCopyPass::~GPUCopyPass() = default;
