#include "create_pipeline.h"

#include <format>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/pipeline_names.h>
#include <shaders/ui/common_ui.h>
#include <shaders/ui/image_2d.h>
#include <shaders/ui/texture_sequence.h>

#include "graphics_pipeline.h"

using namespace storm;

namespace
{

template <typename VertexInput, typename StageInfo>
auto create_pipeline(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<SDL_Window> const&    window,
    std::shared_ptr<AssetServer> const&   asset_server,
    std::string const&                    vertex_shader,
    std::string const&                    fragment_shader) -> std::shared_ptr<GraphicsPipeline>
{
    auto const vertex_shader_asset   = asset_server->load_shader_file(vertex_shader);
    auto const fragment_shader_asset = asset_server->load_shader_file(fragment_shader);

    return std::make_shared<GraphicsPipeline>(
        device,
        window,
        VertexInput::attributes(),
        VertexInput::descriptions(),
        vertex_shader_asset,
        StageInfo::VERTEX,
        fragment_shader_asset,
        StageInfo::FRAGMENT);
}

}  // namespace

namespace storm::pipeline
{

auto create_by_name(
    std::shared_ptr<SDL_GPUDevice> const& device,
    std::shared_ptr<SDL_Window> const&    window,
    std::shared_ptr<AssetServer> const&   asset_server,
    entt::hashed_string const&            name) -> std::shared_ptr<GraphicsPipeline>
{
    std::string const ui_vertex_shader   = std::format("ui/{}_vs", name.data());
    std::string const ui_fragment_shader = std::format("ui/{}_fs", name.data());

    switch (name.value()) {
    case COMMON_UI_PIPELINE.value():
        return create_pipeline<shaders::common_ui::VertexInput, shaders::common_ui::StageInfo>(
            device, window, asset_server, ui_vertex_shader, ui_fragment_shader);
    case IMAGE_2D_PIPELINE.value():
        return create_pipeline<shaders::image_2d::VertexInput, shaders::image_2d::StageInfo>(
            device, window, asset_server, ui_vertex_shader, ui_fragment_shader);
    case TEXTURE_SEQUENCE_PIPELINE.value():
        return create_pipeline<shaders::texture_sequence::VertexInput, shaders::texture_sequence::StageInfo>(
            device, window, asset_server, ui_vertex_shader, ui_fragment_shader);
    default: break;
    }

    return nullptr;
}

}  // namespace storm::pipeline
