#pragma once

#include <memory>

#include <entt/core/fwd.hpp>

#include "graphics_pipeline.h"

namespace storm
{
class AssetServer;
class IConfigLoader;
}  // namespace storm

struct SDL_GPUDevice;
struct SDL_Window;

namespace storm::pipeline
{

auto create(
    std::shared_ptr<SDL_GPUDevice> const&          device,
    std::shared_ptr<SDL_Window> const&             window,
    std::shared_ptr<AssetServer> const&            asset_server,
    std::shared_ptr<IConfigLoader> const&          config_loader,
    std::vector<shaders::VertexAttribute> const&   vertex_attributes,
    std::vector<shaders::VertexDescription> const& vertex_descriptions,
    std::string const&                             vertex_shader,
    std::string const&                             fragment_shader) -> std::shared_ptr<GraphicsPipeline>;

auto create_from_technique(
    std::shared_ptr<SDL_GPUDevice> const&          device,
    std::shared_ptr<SDL_Window> const&             window,
    std::shared_ptr<AssetServer> const&            asset_server,
    std::shared_ptr<IConfigLoader> const&          config_loader,
    std::vector<shaders::VertexAttribute> const&   vertex_attributes,
    std::vector<shaders::VertexDescription> const& vertex_descriptions,
    std::string const&                             vertex_shader,
    std::string const&                             technique) -> std::shared_ptr<GraphicsPipeline>;

template <typename VertexInput>
auto create(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<SDL_Window> const&    window,
    std::shared_ptr<AssetServer> const&   asset_server,
    std::shared_ptr<IConfigLoader> const& config_loader,
    std::string const&                    vertex_shader,
    std::string const&                    fragment_shader) -> std::shared_ptr<GraphicsPipeline>
{
    return create(
        device,
        window,
        asset_server,
        config_loader,
        VertexInput::attributes(),
        VertexInput::descriptions(),
        vertex_shader,
        fragment_shader);
}

template <typename VertexInput>
auto create_from_technique(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<SDL_Window> const&    window,
    std::shared_ptr<AssetServer> const&   asset_server,
    std::shared_ptr<IConfigLoader> const& config_loader,
    std::string const&                    vertex_shader,
    std::string const&                    technique) -> std::shared_ptr<GraphicsPipeline>
{
    return create_from_technique(
        device, window, asset_server, config_loader, VertexInput::attributes(), VertexInput::descriptions(), vertex_shader, technique);
}

}  // namespace storm::pipeline
