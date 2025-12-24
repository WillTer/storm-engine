#pragma once

#include "i_renderer_next.h"

namespace storm
{

class RendererNext final: virtual public IRendererNext
{
public:
    RendererNext();
    ~RendererNext() override;
};

}  // namespace storm
