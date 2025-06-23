#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>

#include "i_renderer_next.h"

struct SDL_GPUDevice;
struct SDL_GPUGraphicsPipeline;
struct SDL_GPUSampler;

namespace storm
{

class RendererSDL final: virtual public IRendererNext
{
public:
    RendererSDL();
    ~RendererSDL() override;

    void bind_window(InternalWindowType const& window) override;
    void unbind_window(InternalWindowType const& window) override;

    void init() override;

private:
    SDL_Window* m_window = nullptr;

    std::shared_ptr<SDL_GPUDevice>           m_device   = nullptr;
    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;
    std::shared_ptr<SDL_GPUSampler>          m_sampler  = nullptr;
    std::shared_ptr<SDL_GPUTexture>          m_texture  = nullptr;

    std::shared_ptr<SDL_GPUBuffer> m_vertex_buffer = nullptr;
    std::shared_ptr<SDL_GPUBuffer> m_index_buffer  = nullptr;
};

}  // namespace storm
