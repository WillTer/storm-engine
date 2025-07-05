#pragma once

#include <memory>
#include <span>

#include <entt/core/fwd.hpp>
#include <libs/renderer_next/types.h>
#include <shaders/info.h>

#include "concepts.h"
#include "gpu_render_pass.h"
#include "graphics_pipeline.h"
#include "sdl_fwd.h"

namespace storm
{

class IConfigLoader;
class AssetServer;
struct ShaderAsset;
struct TxFileHeader;

class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;
class GPUCommandBuffer;
class GPUCopyPass;

class RendererService final
{
public:
    RendererService(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader);
    ~RendererService();

    void bind_window(std::shared_ptr<SDL_Window> const& raw_window);
    void unbind_window(std::shared_ptr<SDL_Window> const& raw_window);

    [[nodiscard]] auto create_pipeline(entt::hashed_string const& name) -> std::shared_ptr<GraphicsPipeline>;

    template <typename T>
        requires std::is_same_v<std::remove_cv_t<T>, uint32_t>
    [[nodiscard]] auto create_index_buffer(std::span<T> const& buffer) -> std::unique_ptr<GPUIndexBuffer>
    {
        return create_index_buffer(buffer.size());
    }

    template <typename T>
        requires has_shader_layout<T>
    [[nodiscard]] auto create_vertex_buffer(std::span<T> const& buffer) -> std::unique_ptr<GPUVertexBuffer>
    {
        return create_vertex_buffer(buffer.size(), sizeof(buffer[0]));
    }

    [[nodiscard]] auto create_texture(entt::hashed_string const& name, TxFileHeader const& file_header) -> std::shared_ptr<GPUTexture>;
    [[nodiscard]] auto create_texture_target(uint32_t width = 0, uint32_t height = 0) -> std::unique_ptr<GPUTexture>;

    [[nodiscard]] auto create_index_buffer(size_t index_count) -> std::unique_ptr<GPUIndexBuffer>;
    [[nodiscard]] auto create_vertex_buffer(size_t vertex_count, size_t vertex_type_size) -> std::unique_ptr<GPUVertexBuffer>;

    auto acquire_command_buffer() const -> std::unique_ptr<GPUCommandBuffer>;

    auto get_viewport() const -> FRect;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm
