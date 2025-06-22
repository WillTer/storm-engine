#pragma once

#include "i_renderer_next.h"

namespace storm
{

class RendererRlgl final: virtual public IRendererNext
{
public:
    RendererRlgl();
    ~RendererRlgl() override;

    void init() override;
};

}  // namespace storm
