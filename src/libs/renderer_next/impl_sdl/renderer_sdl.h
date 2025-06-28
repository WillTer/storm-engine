#pragma once

#include <memory>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/types.h>

#include "graphics_pipeline.h"

namespace storm
{

template <typename T>
concept has_shader_layout = std::is_standard_layout_v<T> && requires() {
    { T::attributes() } -> std::same_as<std::vector<VertexAttribute>>;
    { T::descriptions() } -> std::same_as<std::vector<VertexDescription>>;
};

class AssetServer;
struct ShaderAsset;
struct TextureAsset;

class IConfigLoader;
class GPUVertexBuffer;
class GPUIndexBuffer;
class GPUTexture;

class RendererService final
{
public:
    RendererService(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader);
    ~RendererService();

    void bind_window(SDL_Window* raw_window);
    void unbind_window(SDL_Window* raw_window);

    [[nodiscard]] std::unique_ptr<GPUTexture> load_texture(TextureAsset const& asset);
    [[nodiscard]] std::unique_ptr<GPUTexture> create_texture_target();
    [[nodiscard]] GPUTexture&                 get_render_target();

    template <typename VertexType>
        requires has_shader_layout<VertexType>
    [[nodiscard]] std::unique_ptr<GraphicsPipeline> create_pipeline(
        ShaderAsset const& vertex_shader_asset,
        ShaderInfo const&  vertex_shader_info,
        ShaderAsset const& fragment_shader_asset,
        ShaderInfo const&  fragment_shader_info)
    {
        return create_pipeline(
            VertexType::attributes(),
            VertexType::descriptions(),
            vertex_shader_asset,
            vertex_shader_info,
            fragment_shader_asset,
            fragment_shader_info);
    }

    template <typename IndexType>
        requires(std::is_same_v<IndexType, uint16_t> || std::is_same_v<IndexType, uint32_t>)
    [[nodiscard]] std::unique_ptr<GPUIndexBuffer> load_index_buffer(std::vector<IndexType> const& buffer)
    {
        return load_index_buffer(buffer.data(), buffer.size(), sizeof(IndexType));
    }

    template <typename VertexType>
        requires has_shader_layout<VertexType>
    [[nodiscard]] std::unique_ptr<GPUVertexBuffer> load_vertex_buffer(std::vector<VertexType> const& buffer)
    {
        return load_vertex_buffer(buffer.data(), buffer.size());
    }

    template <typename DataType>
    void push_vertex_uniform_data(uint32_t slot, DataType const& data)
    {
        push_vertex_uniform_data(slot, &data, sizeof(data));
    }

    void push_vertex_uniform_data(uint32_t slot, void const* data, uint32_t data_size);

    template <typename DataType>
    void push_fragment_uniform_data(uint32_t slot, DataType const& data)
    {
        push_fragment_uniform_data(slot, &data, sizeof(data));
    }

    void push_fragment_uniform_data(uint32_t slot, void const* data, uint32_t data_size);

    void start_frame();
    void end_frame();

    void start_render_pass();
    void end_render_pass();

    FRect get_viewport() const;

    SDL_GPUTextureFormat get_spawchain_texture_format() const;

    std::shared_ptr<SDL_GPUDevice> const&        get_device() const;
    std::shared_ptr<SDL_GPUCommandBuffer> const& get_current_command_buffer() const;
    std::shared_ptr<SDL_GPURenderPass> const&    get_current_render_pass() const;

    void start_render_pass(std::shared_ptr<SDL_GPURenderPass> const& texture_render_pass);

private:
    [[nodiscard]] std::unique_ptr<GraphicsPipeline> create_pipeline(
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info);

    [[nodiscard]] std::unique_ptr<GPUIndexBuffer>  load_index_buffer(void const* data, uint32_t data_size, uint32_t index_size);
    [[nodiscard]] std::unique_ptr<GPUVertexBuffer> load_vertex_buffer(void const* data, uint32_t data_size);

    SDL_Window* m_window = nullptr;

    SDL_GPUViewport m_viewport = {};

    std::shared_ptr<SDL_GPUDevice> m_device = nullptr;

    std::shared_ptr<SDL_GPUCommandBuffer> m_current_command_buffer = nullptr;
    std::shared_ptr<SDL_GPURenderPass>    m_current_render_pass    = nullptr;

    std::shared_ptr<GPUTexture> m_render_target = nullptr;

    SDL_GPUTexture* m_swapchain_texture = nullptr;
};

}  // namespace storm
