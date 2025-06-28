#include "draw_texture_with_fade.h"

#include <algorithm>
#include <cassert>
#include <limits>

using namespace storm;

DrawTextureWithFade::DrawTextureWithFade() {}

DrawTextureWithFade::~DrawTextureWithFade() = default;

void DrawTextureWithFade::update(uint64_t const delta_time)
{
    constexpr float ms_to_s = 0.001F;  // Multiplier to convert milliseconds to seconds

    m_fade_alpha = std::clamp(m_fade_alpha + (delta_time * ms_to_s * m_fade_speed), 0.0F, 1.0F);
    m_color.a    = m_fade_alpha;

    m_is_fade_finished = (m_fade_speed < 0.0F && m_fade_alpha <= std::numeric_limits<float>::epsilon())  // Fade out
        || (m_fade_speed > 0.0F && (m_fade_alpha - 1.0F) <= std::numeric_limits<float>::epsilon());      // Fade in
}

void DrawTextureWithFade::start_fade(float alpha, float speed)
{
    m_fade_alpha       = std::clamp(alpha, 0.0F, 1.0F);
    m_fade_speed       = speed;
    m_is_fade_finished = false;
}

bool DrawTextureWithFade::is_fade_finished() const
{
    return m_is_fade_finished;
}
