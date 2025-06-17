#pragma once

#include "i_renderer_service_next.h"

namespace storm
{

class RendererServiceNext final: virtual public IRendererServiceNext
{
public:
    RendererServiceNext();
    ~RendererServiceNext() override;

    bool Init() override;
};

}  // namespace storm