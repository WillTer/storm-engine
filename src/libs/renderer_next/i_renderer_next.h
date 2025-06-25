#pragma once

#include <any>
#include <filesystem>

#include "i_pipeline.h"
#include "vertex.h"

namespace storm
{

struct TextureAsset;
struct ShaderAsset;

class ITexture;
class IBuffer;

template <typename T>
concept is_vertex_for_pipeline = std::is_standard_layout_v<T> && requires() {
    { T::attributes() } -> std::same_as<std::vector<VertexAttribute>>;
    { T::descriptions() } -> std::same_as<std::vector<VertexDescription>>;
};

class RendererNext
{
public:
    virtual ~RendererNext() = default;

    virtual void bind_window(std::any const& window_handler_internal)   = 0;
    virtual void unbind_window(std::any const& window_handler_internal) = 0;

    [[nodiscard]] virtual std::unique_ptr<ITexture> load_texture(TextureAsset const& asset) = 0;

    template <typename VertexType>
        requires is_vertex_for_pipeline<VertexType>
    [[nodiscard]] std::unique_ptr<IPipeline> create_pipeline(
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

    [[nodiscard]] virtual std::unique_ptr<IPipeline> create_pipeline(
        std::vector<VertexAttribute> const&   vertex_attributes,
        std::vector<VertexDescription> const& vertex_descriptions,
        ShaderAsset const&                    vertex_shader_asset,
        ShaderInfo const&                     vertex_shader_info,
        ShaderAsset const&                    fragment_shader_asset,
        ShaderInfo const&                     fragment_shader_info) = 0;

    [[nodiscard]] virtual std::unique_ptr<IBuffer> load_index_buffer(std::vector<uint16_t> const& buffer) = 0;

    template <typename VertexType>
        requires is_vertex_for_pipeline<VertexType>
    [[nodiscard]] std::unique_ptr<IBuffer> load_vertex_buffer(std::vector<VertexType> const& buffer)
    {
        return load_vertex_buffer(buffer.data(), static_cast<uint32_t>(buffer.size() * sizeof(VertexType)));
    }

    [[nodiscard]] virtual std::unique_ptr<IBuffer> load_vertex_buffer(void const* data, uint32_t data_size) = 0;

    virtual void start_frame() = 0;
    virtual void end_frame()   = 0;

    virtual void start_pass() = 0;
    virtual void end_pass()   = 0;
};

}  // namespace storm
