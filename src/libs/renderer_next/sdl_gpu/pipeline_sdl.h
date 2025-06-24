#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <libs/renderer_next/i_pipeline.h>

namespace storm
{

struct ShaderAsset;
class RendererSDL;

template <typename T>
auto get_vertex_attributes() -> std::vector<SDL_GPUVertexAttribute>
{
    throw std::runtime_error("Not implemented");
}

template <>
auto get_vertex_attributes<Position>() -> std::vector<SDL_GPUVertexAttribute>;

template <>
auto get_vertex_attributes<PositionTexture>() -> std::vector<SDL_GPUVertexAttribute>;

template <>
auto get_vertex_attributes<PositionTextureColor>() -> std::vector<SDL_GPUVertexAttribute>;

class PipelineSDL final: virtual public IPipeline
{
    PipelineSDL(
        RendererSDL&                               renderer,
        std::shared_ptr<SDL_GPUCopyPass> const&    copy_pass,
        ShaderAsset const&                         vertex_shader_asset,
        ShaderInfo const&                          vertex_shader_info,
        ShaderAsset const&                         fragment_shader_asset,
        ShaderInfo const&                          fragment_shader_info,
        void const*                                vertex_data,
        uint32_t                                   vertex_count,
        uint32_t                                   vertex_size,
        std::vector<SDL_GPUVertexAttribute> const& vertex_attributes,
        std::vector<uint16_t> const&               indices);

public:
    template <typename T>
    PipelineSDL(
        RendererSDL&                            renderer,
        std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
        ShaderAsset const&                      vertex_shader_asset,
        ShaderInfo const&                       vertex_shader_info,
        ShaderAsset const&                      fragment_shader_asset,
        ShaderInfo const&                       fragment_shader_info,
        std::vector<T> const&                   vertices,
        std::vector<uint16_t> const&            indices)
        : PipelineSDL(
              renderer,
              copy_pass,
              vertex_shader_asset,
              vertex_shader_info,
              fragment_shader_asset,
              fragment_shader_info,
              vertices.data(),
              static_cast<uint32_t>(vertices.size()),
              sizeof(T),
              get_vertex_attributes<T>(),
              indices)
    {
    }

    ~PipelineSDL() override;

    void present() const override;

private:
    RendererSDL& m_renderer;

    std::shared_ptr<SDL_GPUGraphicsPipeline> m_pipeline = nullptr;

    std::shared_ptr<SDL_GPUBuffer> m_vertex_buffer = nullptr;
    std::shared_ptr<SDL_GPUBuffer> m_index_buffer  = nullptr;

    uint32_t m_index_count = 0;
};

}  // namespace storm
