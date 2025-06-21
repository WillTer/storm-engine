#pragma once

#include <libs/window/os_window.hpp>

namespace storm
{

class IRendererNext
{
public:
    virtual ~IRendererNext() = default;

    virtual void bind_window(std::shared_ptr<OSWindow> const& window) = 0;
};

}  // namespace storm
