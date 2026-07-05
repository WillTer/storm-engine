#pragma once

#include <memory>
#include <type_traits>
#include <vector>

#include <shaders/cpp/vertex.h>

#include "sdl_fwd.h"

namespace storm
{

struct BufferUpdateInfo {
    uint32_t offset;
    uint32_t size;
};

template <typename T>
concept has_shader_layout = std::is_standard_layout_v<T> && requires() {
    { T::attributes() } -> std::same_as<std::vector<shaders::VertexAttribute>>;
    { T::descriptions() } -> std::same_as<std::vector<shaders::VertexDescription>>;
};

template <typename T>
concept can_upload = requires(T& t, std::shared_ptr<SDL_GPUCopyPass> const& copy_pass) {
    { t.upload(copy_pass) };
};

template <typename T, typename V>
concept can_update = requires(
    T const&                                t,
    std::shared_ptr<SDL_GPUDevice> const&   device,
    std::shared_ptr<SDL_GPUCopyPass> const& copy_pass,
    std::vector<BufferUpdateInfo> const&    update_info,
    std::vector<V> const&                   data,
    size_t                                  stride) {
    { t.update(device, copy_pass, update_info, data, stride) };
};

template <typename T>
concept can_bind_to_render_pass = requires(T const& t, std::shared_ptr<SDL_GPURenderPass> const& render_pass) {
    { t.bind_to_render_pass(render_pass) };
};

template <typename T>
concept can_draw_on_render_pass = requires(T const& t, std::shared_ptr<SDL_GPURenderPass> const& render_pass) {
    { t.draw(render_pass) };
};

}  // namespace storm
