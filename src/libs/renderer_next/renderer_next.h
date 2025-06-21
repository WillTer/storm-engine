#pragma once

#include <memory>

#include "i_renderer_next.h"

struct SDL_GPUDevice;

namespace storm
{

class RendererNext final: virtual public IRendererNext
{
public:
    RendererNext();
    ~RendererNext() override;

    void bind_window(InternalWindowType const& window) override;
    void unbind_window(InternalWindowType const& window) override;

private:
    std::shared_ptr<SDL_GPUDevice> m_device = nullptr;
};

}  // namespace storm
