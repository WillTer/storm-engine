#pragma once

#include <memory>
#include <string>

#include <entt/core/fwd.hpp>
#include <libs/renderer_next/types.h>
#include <shaders/info.h>

#include "concepts.h"
#include "gpu_render_pass.h"
#include "gpu_texture.h"
#include "graphics_pipeline.h"
#include "sdl_fwd.h"

namespace storm
{

class IConfigLoader;
class AssetServer;

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
        requires has_shader_layout<T>
    [[nodiscard]] auto create_vertex_buffer(std::vector<T> const& buffer) -> std::shared_ptr<GPUVertexBuffer>
    {
        return create_vertex_buffer(buffer.data(), buffer.size(), sizeof(buffer[0]));
    }

    [[nodiscard]] auto create_texture(std::string const& file, GPUTexture::AddressMode address_mode = GPUTexture::AddressMode::Repeat)
        -> std::shared_ptr<GPUTexture>;
    [[nodiscard]] auto create_texture_target(uint32_t width = 0, uint32_t height = 0) -> std::unique_ptr<GPUTexture>;

    [[nodiscard]] auto create_index_buffer(std::vector<uint32_t> const& indices) -> std::shared_ptr<GPUIndexBuffer>;
    [[nodiscard]] auto create_vertex_buffer(void const* data, size_t vertex_count, size_t vertex_type_size)
        -> std::shared_ptr<GPUVertexBuffer>;

    void upload_pending_data(storm::GPUCopyPass const& copy_pass);

    auto acquire_command_buffer() const -> std::unique_ptr<GPUCommandBuffer>;

    auto get_viewport() const -> FRect;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace storm
