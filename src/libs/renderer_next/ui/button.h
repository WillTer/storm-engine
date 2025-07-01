#pragma once

#include <memory>

#include <libs/renderer_next/types.h>

namespace storm
{

class GPUCopyPass;
class GPURenderPass;
class Picture;

class Button final
{
public:
    Button(std::unique_ptr<Picture>&& left, std::unique_ptr<Picture>&& middle, std::unique_ptr<Picture>&& right);
    ~Button();

    void update(GPUCopyPass const& copy_pass, uint64_t delta_time);
    void draw(GPURenderPass const& render_pass) const;

    void set_rect(storm::FRect const& rect);
    void set_screen_rect(storm::FRect const& rect);
    void set_diffuse_color(storm::Color const& color);

private:
    std::unique_ptr<Picture> m_left;
    std::unique_ptr<Picture> m_middle;
    std::unique_ptr<Picture> m_right;
};

}  // namespace storm
