#pragma once

#include <libs/window/i_window.h>

namespace storm
{

class IRendererNext
{
public:
    virtual ~IRendererNext() = default;

    virtual void init() = 0;
};

}  // namespace storm
