#pragma once

#include <memory>
#include <vector>

#include <libs/renderer_next/types.h>

#include "concepts.h"
#include "sdl_fwd.h"

namespace storm
{

enum class LoadOp { Load, Clear, DontCare };
enum class StoreOp { Store, DontCare, Resolve, ResolveStore };

struct ColorTargetInfo {
    SDL_GPUTexture* target_texture;
    FColor          clear_color;
    LoadOp          load_op;
    StoreOp         store_op;
};

class GPURenderPass final
{
public:
    GPURenderPass(
        std::shared_ptr<SDL_GPUCommandBuffer> const& cmd_buffer,
        std::vector<ColorTargetInfo> const&          color_targets,
        std::optional<storm::FRect> const&           viewport);

    ~GPURenderPass();

    void bind(can_bind_to_render_pass auto const& object) const
    {
        object.bind_to_render_pass(m_pass);
    }

    void draw(can_draw_on_render_pass auto const& object) const
    {
        object.draw(m_pass);
    }

    template <typename DataType>
    void push_vertex_uniform_data(uint32_t slot, DataType const& data) const
    {
        push_vertex_uniform_data(slot, &data, sizeof(data));
    }

    void push_vertex_uniform_data(uint32_t slot, void const* data, size_t data_size) const;

    template <typename DataType>
    void push_fragment_uniform_data(uint32_t slot, DataType const& data) const
    {
        push_fragment_uniform_data(slot, &data, sizeof(data));
    }

    void push_fragment_uniform_data(uint32_t slot, void const* data, size_t data_size) const;

private:
    std::shared_ptr<SDL_GPURenderPass>    m_pass       = nullptr;
    std::shared_ptr<SDL_GPUCommandBuffer> m_cmd_buffer = nullptr;
};

}  // namespace storm
