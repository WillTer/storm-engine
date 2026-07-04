#pragma once

#include <entt/core/hashed_string.hpp>

namespace storm
{

using namespace entt::literals;

constexpr entt::hashed_string COMMON_UI_PIPELINE        = "common_ui"_hs;
constexpr entt::hashed_string IMAGE_2D_PIPELINE         = "image_2d"_hs;
constexpr entt::hashed_string TEXTURE_SEQUENCE_PIPELINE = "texture_sequence"_hs;
constexpr entt::hashed_string FILL_RECTANGLE_PIPELINE   = "rectangle"_hs;
constexpr entt::hashed_string WIRE_RECTANGLE_PIPELINE   = "wire_rectangle"_hs;
constexpr entt::hashed_string FONT_NORMAL_PIPELINE      = "font_normal"_hs;
constexpr entt::hashed_string IMAGE_2D_BRIGHT_PIPELINE  = "image_2d_bright"_hs;

}  // namespace storm
