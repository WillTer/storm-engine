#include "button.h"

#include <cassert>

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/renderer_next/impl_sdl/gpu_command_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_index_buffer.h>
#include <libs/renderer_next/impl_sdl/gpu_texture.h>
#include <libs/renderer_next/impl_sdl/gpu_vertex_buffer.h>
#include <libs/renderer_next/impl_sdl/renderer_sdl.h>

#include "picture.h"

using namespace storm;

Button::Button(std::unique_ptr<Picture>&& left, std::unique_ptr<Picture>&& middle, std::unique_ptr<Picture>&& right)
    : m_left(std::move(left))
    , m_middle(std::move(middle))
    , m_right(std::move(right))
{
    assert(m_left);
    assert(m_middle);
    assert(m_right);
}

Button::~Button() = default;

void Button::update(GPUCopyPass const& copy_pass, uint64_t const delta_time)
{
    m_left->update(copy_pass, delta_time);
    m_middle->update(copy_pass, delta_time);
    m_right->update(copy_pass, delta_time);
}

void Button::draw(GPURenderPass const& render_pass) const
{
    m_middle->draw(render_pass);
    m_left->draw(render_pass);
    m_right->draw(render_pass);
}

void Button::set_rect(storm::FRect const& rect)
{
    auto const [left_w, left_h] = m_left->get_dimensions();
    auto const left_middle      = static_cast<float>(rect.left + left_w);

    auto const [right_w, right_h] = m_right ? m_right->get_dimensions() : m_left->get_dimensions();
    auto const right_middle       = static_cast<float>(rect.right - right_w);

    auto const left_rect = storm::FRect {
        .left   = static_cast<float>(rect.left),
        .top    = static_cast<float>(rect.top),
        .right  = left_middle,
        .bottom = static_cast<float>(rect.bottom),
    };

    auto const right_rect = storm::FRect {
        .left   = right_middle,
        .top    = static_cast<float>(rect.top),
        .right  = static_cast<float>(rect.right),
        .bottom = static_cast<float>(rect.bottom),
    };

    auto const middle_rect = storm::FRect {
        .left   = left_middle,
        .top    = static_cast<float>(rect.top),
        .right  = right_middle,
        .bottom = static_cast<float>(rect.bottom),
    };

    m_left->set_rect(left_rect);
    m_middle->set_rect(middle_rect);
    m_right->set_rect(right_rect);
}

void Button::set_screen_rect(storm::FRect const& rect)
{
    m_left->set_screen_rect(rect);
    m_middle->set_screen_rect(rect);
    m_right->set_screen_rect(rect);
}

void Button::set_diffuse_color(storm::Color const& color)
{
    m_left->set_diffuse_color(color);
    m_middle->set_diffuse_color(color);
    m_right->set_diffuse_color(color);
}

auto Button::get_middle_rect() const -> storm::FRect
{
    return m_middle->get_rect();
}
