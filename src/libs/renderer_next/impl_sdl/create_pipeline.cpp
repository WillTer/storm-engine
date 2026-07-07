#include "create_pipeline.h"

#include <format>

#include <libs/asset_server/asset_server.h>
#include <libs/config/i_config_loader.h>
#include <libs/config/ini_file.h>
#include <libs/config/technique.h>
#include <libs/core/core.h>

#include "graphics_pipeline.h"

using namespace storm;

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
    std::string const&                             fragment_shader) -> std::shared_ptr<GraphicsPipeline>
{
    auto const vertex_shader_asset   = asset_server->load_shader_file(vertex_shader);
    auto const fragment_shader_asset = asset_server->load_shader_file(fragment_shader);

    auto const& vertex_shader_meta =
        config_loader->open_config_cached(asset_server->get_asset_dir<ShaderAsset>() / std::format("{}_meta.ini", vertex_shader), false);
    auto const& fragment_shader_meta =
        config_loader->open_config_cached(asset_server->get_asset_dir<ShaderAsset>() / std::format("{}_meta.ini", fragment_shader), false);

    return std::make_shared<GraphicsPipeline>(
        device,
        window,
        vertex_attributes,
        vertex_descriptions,
        vertex_shader_asset,
        vertex_shader_meta,
        fragment_shader_asset,
        fragment_shader_meta,
        std::nullopt);
}

auto create(
    std::shared_ptr<SDL_GPUDevice> const&          device,
    std::shared_ptr<SDL_Window> const&             window,
    std::shared_ptr<AssetServer> const&            asset_server,
    std::shared_ptr<IConfigLoader> const&          config_loader,
    std::vector<shaders::VertexAttribute> const&   vertex_attributes,
    std::vector<shaders::VertexDescription> const& vertex_descriptions,
    std::string const&                             vertex_shader,
    PipelineInfo const&                            info) -> std::shared_ptr<GraphicsPipeline>
{
    auto const vertex_shader_asset   = asset_server->load_shader_file(vertex_shader);
    auto const fragment_shader_asset = asset_server->load_shader_file(info.fragment_shader);

    auto const& vertex_shader_meta =
        config_loader->open_config_cached(asset_server->get_asset_dir<ShaderAsset>() / std::format("{}_meta.ini", vertex_shader), false);
    auto const& fragment_shader_meta = config_loader->open_config_cached(
        asset_server->get_asset_dir<ShaderAsset>() / std::format("{}_meta.ini", info.fragment_shader), false);

    return std::make_shared<GraphicsPipeline>(
        device,
        window,
        vertex_attributes,
        vertex_descriptions,
        vertex_shader_asset,
        vertex_shader_meta,
        fragment_shader_asset,
        fragment_shader_meta,
        info);
}

}  // namespace storm::pipeline
