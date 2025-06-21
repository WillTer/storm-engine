#pragma once

#include <memory>

#include "i_renderer_next.h"

struct SDL_GPUDevice;
struct SDL_GPUGraphicsPipeline;

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
};

}  // namespace storm
