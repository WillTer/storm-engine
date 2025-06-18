#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <utility>

namespace storm
{

template <typename T>
struct PointBase {
    T x;
    T y;

    constexpr PointBase() : x(0), y(0) {}
    constexpr PointBase(T x, T y) : x(x), y(y) {}
    constexpr PointBase(std::pair<T, T> const& p) : x(p.first), y(p.second) {}

    constexpr std::pair<int, int> as_pair() const
    {
        return std::make_pair(x, y);
    }

    // Wait until C++26 when std::sqrt will be constexpr
    double distance(PointBase const& other) const
    {
        return std::sqrt((other.x - x) * (other.x - x)) + ((other.y - y) * (other.y - y));
    }

    constexpr PointBase& operator+=(PointBase const& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr PointBase& operator-=(PointBase const& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    friend constexpr PointBase operator+(PointBase lhs, PointBase const& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend constexpr PointBase operator-(PointBase lhs, PointBase const& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    constexpr auto operator<=>(PointBase const&) const = default;
};

using Point  = PointBase<int>;
using FPoint = PointBase<float>;

template <typename T>
struct RectBase {
    T left;
    T top;
    T right;
    T bottom;

    constexpr RectBase() : left(0), top(0), right(0), bottom(0) {}
    constexpr RectBase(T left, T top, T right, T bottom) : left(left), top(top), right(right), bottom(bottom) {}

    constexpr RectBase(PointBase<T> const& top_left, PointBase<T> const& bottom_right)
        : left(top_left.x)
        , top(top_left.y)
        , right(bottom_right.x)
        , bottom(bottom_right.y)
    {
    }

    constexpr RectBase(PointBase<T> const& top_left, int width, int height)
        : left(top_left.x)
        , top(top_left.y)
        , right(top_left.x + width)
        , bottom(top_left.y + height)
    {
    }

    constexpr bool intersect(RectBase const& other) const
    {
        return left <= other.right && right >= other.left && top <= other.bottom && bottom >= other.top;
    }

    constexpr PointBase<T> center() const
    {
        return PointBase<T>(left + (width() / 2), top + (height() / 2));
    }

    constexpr bool contains_point(PointBase<T> const& point) const
    {
        return point.x >= left && point.y >= top && point.x <= right && point.y <= bottom;
    }

    constexpr int width() const
    {
        return right - left;
    }

    constexpr int height() const
    {
        return bottom - top;
    }

    constexpr auto operator<=>(RectBase const&) const = default;
};

using Rect  = RectBase<int>;
using FRect = RectBase<float>;

struct Color {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;

    constexpr std::tuple<double, double, double, double> normalize() const
    {
        constexpr double max_value = static_cast<double>(std::numeric_limits<uint8_t>::max());
        return std::make_tuple(a / max_value, r / max_value, g / max_value, b / max_value);
    }

    constexpr Color grayscale() const
    {
        auto const [_, rn, gn, bn] = normalize();
        auto const y               = static_cast<uint8_t>((0.299 * rn + 0.587 * gn + 0.114 * bn) * 255.0);

        return Color {a, y, y, y};
    }

    constexpr uint32_t to_hex() const
    {
        auto hex_bytes = std::bit_cast<std::array<uint8_t, 4>>(*this);
        if constexpr (std::endian::native == std::endian::little) { std::ranges::reverse(hex_bytes); }

        return std::bit_cast<uint32_t>(hex_bytes);
    }

    constexpr static Color from_hex(uint32_t const hex)
    {
        auto hex_bytes = std::bit_cast<std::array<uint8_t, 4>>(hex);
        if constexpr (std::endian::native == std::endian::little) { std::ranges::reverse(hex_bytes); }

        return std::bit_cast<Color>(hex_bytes);
    }

    constexpr auto operator<=>(Color const&) const = default;
};

}  // namespace storm