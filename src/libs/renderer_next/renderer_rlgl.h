#pragma once

#include <raylib.h>

#include "i_renderer_next.h"

namespace storm
{

class RendererRlgl final: virtual public IRendererNext
{
public:
    RendererRlgl();
    ~RendererRlgl() override;

    void init() override;
    void draw() override;

private:
    Texture2D m_loading;
};

}  // namespace storm
