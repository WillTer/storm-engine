#pragma once

#include <entt/core/hashed_string.hpp>

namespace storm
{

using namespace entt::literals;

constexpr entt::hashed_string COMMON_UI_PIPELINE        = "common_ui"_hs;
constexpr entt::hashed_string IMAGE_2D_PIPELINE         = "image_2d"_hs;
constexpr entt::hashed_string TEXTURE_SEQUENCE_PIPELINE = "texture_sequence"_hs;
constexpr entt::hashed_string COLORED_RECT_PIPELINE     = "colored_rect"_hs;

}  // namespace storm
