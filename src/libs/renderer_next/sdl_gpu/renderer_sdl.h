#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_renderer_next.h>

#include "pipeline_sdl.h"

namespace storm
{

class AssetServer;
class IConfigLoader;

class RendererSDL final: virtual public RendererNext
{
public:
    RendererSDL(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader);
    ~RendererSDL() override;

    void bind_window(std::any const& window_raw) override;
    void unbind_window(std::any const& window_raw) override;

    [[nodiscard]] std::unique_ptr<ITexture> load_texture(TextureAsset const& asset) override;

    [[nodiscard]] std::unique_ptr<IPipeline> create_pipeline(
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info) override;

    [[nodiscard]] std::unique_ptr<IIndexBuffer> load_index_buffer(std::vector<uint16_t> const& buffer) override;

    [[nodiscard]] std::unique_ptr<IBuffer> load_vertex_buffer(void const* data, uint32_t data_size) override;

    void start_frame() override;
    void end_frame() override;

    void start_pass() override;
    void end_pass() override;

    FRect get_viewport() const override;

    void push_vertex_unform_data(uint32_t slot, void const* data, uint32_t data_size) override;

    SDL_GPUTextureFormat get_spawchain_texture_format() const;

    std::shared_ptr<SDL_GPUDevice> const&     get_device() const;
    std::shared_ptr<SDL_GPURenderPass> const& get_current_render_pass() const;

private:
    std::string m_backend;
    SDL_Window* m_window = nullptr;

    SDL_GPUViewport m_viewport = {};

    std::shared_ptr<SDL_GPUDevice> m_device = nullptr;

    std::shared_ptr<SDL_GPUCommandBuffer> m_current_command_buffer = nullptr;
    std::shared_ptr<SDL_GPURenderPass>    m_current_render_pass    = nullptr;
};

}  // namespace storm
