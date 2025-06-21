#pragma once

#include <variant>

struct SDL_Window;

namespace storm
{

using InternalWindowType = std::variant<SDL_Window*>;

enum class ShaderStage { Vertex, Fragment };

class IRendererNext
{
public:
    virtual ~IRendererNext() = default;

    virtual void bind_window(InternalWindowType const& window)   = 0;
    virtual void unbind_window(InternalWindowType const& window) = 0;

    virtual void init() = 0;
};

}  // namespace storm
