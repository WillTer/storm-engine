#pragma once

#include <memory>

#include <entt/core/fwd.hpp>

namespace storm
{
class AssetServer;
class GraphicsPipeline;
}  // namespace storm

struct SDL_GPUDevice;
struct SDL_Window;

namespace storm::pipeline
{

auto create_by_name(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<SDL_Window> const&    window,
    std::shared_ptr<AssetServer> const&   asset_server,
    entt::hashed_string const&            name) -> std::shared_ptr<GraphicsPipeline>;

}
