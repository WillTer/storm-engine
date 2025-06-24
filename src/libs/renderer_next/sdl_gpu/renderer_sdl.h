#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_renderer_next.h>

namespace storm
{

class AssetServer;
class IConfigLoader;
class RendererSDL final: virtual public IRendererNext
{
public:
    RendererSDL(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader);
    ~RendererSDL() override;

    void bind_window(std::any const& window_handler_internal) override;
    void unbind_window(std::any const& window_handler_internal) override;

    [[nodiscard]] std::unique_ptr<ITexture> load_texture(TextureAsset const& asset) override;

    [[nodiscard]] std::unique_ptr<IPipeline> create_pipeline(
        ShaderInfo const&            vertex_shader,
        ShaderInfo const&            fragment_shader,
        std::vector<Position> const& vertices,
        std::vector<uint16_t> const& indices) override;

    [[nodiscard]] std::unique_ptr<IPipeline> create_pipeline(
        ShaderInfo const&                   vertex_shader,
        ShaderInfo const&                   fragment_shader,
        std::vector<PositionTexture> const& vertices,
        std::vector<uint16_t> const&        indices) override;

    [[nodiscard]] std::unique_ptr<IPipeline> create_pipeline(
        ShaderInfo const&                        vertex_shader,
        ShaderInfo const&                        fragment_shader,
        std::vector<PositionTextureColor> const& vertices,
        std::vector<uint16_t> const&             indices) override;

    void start_frame() override;
    void end_frame() override;

    void start_pass() override;
    void end_pass() override;

    SDL_GPUTextureFormat get_spawchain_texture_format() const;

    std::shared_ptr<SDL_GPUDevice> const&     get_device() const;
    std::shared_ptr<SDL_GPURenderPass> const& get_current_render_pass() const;

private:
    std::string m_backend;
    SDL_Window* m_window = nullptr;

    std::shared_ptr<SDL_GPUDevice> m_device = nullptr;

    std::shared_ptr<SDL_GPUCommandBuffer> m_current_command_buffer = nullptr;
    std::shared_ptr<SDL_GPURenderPass>    m_current_render_pass    = nullptr;
};

}  // namespace storm
