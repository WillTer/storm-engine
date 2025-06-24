#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "i_renderer_next.h"

struct SDL_GPUDevice;
struct SDL_GPUGraphicsPipeline;
struct SDL_GPUSampler;
struct SDL_GPURenderPass;
struct SDL_GPUCommandBuffer;
struct SDL_GPUBuffer;

namespace storm
{

class AssetServer;
class IConfigLoader;
class RendererSDL final: virtual public IRendererNext
{
public:
    RendererSDL(std::shared_ptr<AssetServer> const& asset_server, std::shared_ptr<IConfigLoader> const& config_loader);
    ~RendererSDL() override;

    void bind_window(InternalWindowType const& window) override;
    void unbind_window(InternalWindowType const& window) override;

    std::unique_ptr<ITexture> load_texture(TextureAsset const& asset) override;

    void test_init() override;
    void test_draw() override;

    void start_frame() override;
    void end_frame() override;

    void start_pass() override;
    void end_pass() override;

    std::shared_ptr<SDL_GPUDevice> const&     get_device() const;
    std::shared_ptr<SDL_GPURenderPass> const& get_current_render_pass() const;

private:
    std::string m_backend;
    SDL_Window* m_window = nullptr;

    std::shared_ptr<SDL_GPUDevice>           m_device   = nullptr;
    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;

    std::shared_ptr<SDL_GPUCommandBuffer> m_current_command_buffer = nullptr;
    std::shared_ptr<SDL_GPURenderPass>    m_current_render_pass    = nullptr;

    std::shared_ptr<SDL_GPUBuffer> m_vertex_buffer = nullptr;
    std::shared_ptr<SDL_GPUBuffer> m_index_buffer  = nullptr;
};

}  // namespace storm
