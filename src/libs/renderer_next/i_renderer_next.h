#pragma once

#include <any>
#include <filesystem>

#include "i_pipeline.h"

namespace storm
{

struct TextureAsset;

class ITexture;
class IRendererNext
{
public:
    virtual ~IRendererNext() = default;

    virtual void bind_window(std::any const& window_handler_internal)   = 0;
    virtual void unbind_window(std::any const& window_handler_internal) = 0;

    [[nodiscard]] virtual std::unique_ptr<ITexture> load_texture(TextureAsset const& asset) = 0;

    [[nodiscard]] virtual std::unique_ptr<IPipeline> create_pipeline(
        ShaderInfo const&            vertex_shader,
        ShaderInfo const&            fragment_shader,
        std::vector<Position> const& vertices,
        std::vector<uint16_t> const& indices) = 0;

    [[nodiscard]] virtual std::unique_ptr<IPipeline> create_pipeline(
        ShaderInfo const&                   vertex_shader,
        ShaderInfo const&                   fragment_shader,
        std::vector<PositionTexture> const& vertices,
        std::vector<uint16_t> const&        indices) = 0;

    [[nodiscard]] virtual std::unique_ptr<IPipeline> create_pipeline(
        ShaderInfo const&                        vertex_shader,
        ShaderInfo const&                        fragment_shader,
        std::vector<PositionTextureColor> const& vertices,
        std::vector<uint16_t> const&             indices) = 0;

    virtual void start_frame() = 0;
    virtual void end_frame()   = 0;

    virtual void start_pass() = 0;
    virtual void end_pass()   = 0;
};

}  // namespace storm
