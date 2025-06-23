#pragma once

#include <filesystem>
#include <variant>

struct SDL_Window;

namespace storm
{

using InternalWindowType = std::variant<SDL_Window*>;

enum class ShaderStage { Vertex, Fragment };

struct TextureAsset;
class ITexture;
class IRendererNext
{
public:
    virtual ~IRendererNext() = default;

    virtual void bind_window(InternalWindowType const& window)   = 0;
    virtual void unbind_window(InternalWindowType const& window) = 0;

    virtual std::unique_ptr<ITexture> load_texture(TextureAsset const& asset) = 0;

    virtual void test_init() = 0;
    virtual void test_draw() = 0;

    virtual void start_frame() = 0;
    virtual void end_frame()   = 0;

    virtual void start_pass() = 0;
    virtual void end_pass()   = 0;
};

}  // namespace storm
