#pragma once

#include <memory>

#include <libs/renderer_next/types.h>

namespace storm
{

class GPUCopyPass;
class GPURenderPass;
class Image2D;

class Button final
{
public:
    Button(std::unique_ptr<Image2D>&& left, std::unique_ptr<Image2D>&& middle, std::unique_ptr<Image2D>&& right);
    ~Button();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_rect(storm::FRect const& rect);
    void set_screen_rect(storm::FRect const& rect);
    void set_diffuse_color(storm::Color const& color);

    auto get_middle_rect() const -> storm::FRect;

private:
    std::unique_ptr<Image2D> m_left;
    std::unique_ptr<Image2D> m_middle;
    std::unique_ptr<Image2D> m_right;
};

}  // namespace storm
