#pragma once

#include <memory>
#include <span>

#include <libs/renderer_next/types.h>

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

    template <typename VertexType>
        requires has_shader_layout<VertexType>
    [[nodiscard]] auto create_pipeline(
        ShaderAsset const& vertex_shader_asset,
        ShaderInfo const&  vertex_shader_info,
        ShaderAsset const& fragment_shader_asset,
        ShaderInfo const&  fragment_shader_info) -> std::unique_ptr<GraphicsPipeline>
    {
        return create_pipeline(
            VertexType::attributes(),
            VertexType::descriptions(),
            vertex_shader_asset,
            vertex_shader_info,
            fragment_shader_asset,
            fragment_shader_info);
    }

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

    [[nodiscard]] auto create_texture(TxFileHeader const& file_header) -> std::unique_ptr<GPUTexture>;
    [[nodiscard]] auto create_texture_target() -> std::unique_ptr<GPUTexture>;

    [[nodiscard]] auto create_index_buffer(size_t index_count) -> std::unique_ptr<GPUIndexBuffer>;
    [[nodiscard]] auto create_vertex_buffer(size_t vertex_count, size_t vertex_type_size) -> std::unique_ptr<GPUVertexBuffer>;

    auto acquire_command_buffer() const -> std::unique_ptr<GPUCommandBuffer>;

    auto get_viewport() const -> FRect;
    auto get_viewport_native() const -> SDL_GPUViewport const&;

private:
    [[nodiscard]] auto create_pipeline(
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info) -> std::unique_ptr<GraphicsPipeline>;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm
