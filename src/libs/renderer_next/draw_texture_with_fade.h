#pragma once

#include "draw_texture.h"

namespace storm
{

class DrawTextureWithFade final: virtual public DrawTexture
{
public:
    explicit DrawTextureWithFade();
    ~DrawTextureWithFade() override;

    void update(uint64_t delta_time);

    void start_fade(float alpha, float speed);
    bool is_fade_finished() const;

private:
    float m_fade_alpha = 1.0F;
    float m_fade_speed = 1.0F;

    bool m_is_fade_finished = true;
};

}  // namespace storm
