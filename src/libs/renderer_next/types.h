#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <utility>

namespace storm
{

template <typename T>
struct PointBase2D {
    T x;
    T y;

    constexpr std::pair<T, T> as_pair() const
    {
        return std::make_pair(x, y);
    }

    // Wait until C++26 when std::sqrt will be constexpr
    double distance(PointBase2D const& other) const
    {
        return std::sqrt((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y));
    }

    constexpr PointBase2D& operator+=(PointBase2D const& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr PointBase2D& operator-=(PointBase2D const& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    friend constexpr PointBase2D operator+(PointBase2D lhs, PointBase2D const& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend constexpr PointBase2D operator-(PointBase2D lhs, PointBase2D const& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    constexpr auto operator<=>(PointBase2D const&) const = default;
};

using Point  = PointBase2D<int>;
using FPoint = PointBase2D<float>;

template <typename T>
struct PointBase3D {
    T x;
    T y;
    T z;

    constexpr std::tuple<T, T, T> as_tuple() const
    {
        return std::make_tuple(x, y, z);
    }

    // Wait until C++26 when std::sqrt will be constexpr
    double distance(PointBase3D const& other) const
    {
        return std::sqrt((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y) + (other.z - z) * (other.z - z));
    }

    constexpr PointBase3D& operator+=(PointBase3D const& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr PointBase3D& operator-=(PointBase3D const& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    friend constexpr PointBase3D operator+(PointBase3D lhs, PointBase3D const& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend constexpr PointBase3D operator-(PointBase3D lhs, PointBase3D const& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    constexpr auto operator<=>(PointBase3D const&) const = default;
};

using Point3D  = PointBase3D<int>;
using FPoint3D = PointBase3D<float>;

template <typename T>
struct RectBase {
    T left;
    T top;
    T right;
    T bottom;

    constexpr bool intersect(RectBase const& other) const
    {
        return left <= other.right && right >= other.left && top <= other.bottom && bottom >= other.top;
    }

    constexpr PointBase2D<T> center() const
    {
        return PointBase2D<T>(left + (width() / 2), top + (height() / 2));
    }

    constexpr bool contains_point(PointBase2D<T> const& point) const
    {
        return point.x >= left && point.y >= top && point.x <= right && point.y <= bottom;
    }

    constexpr T width() const
    {
        return right - left;
    }

    constexpr T height() const
    {
        return bottom - top;
    }

    constexpr auto operator<=>(RectBase const&) const = default;
};

using Rect  = RectBase<int>;
using FRect = RectBase<float>;

struct FColor {
    float r;
    float g;
    float b;
    float a;  // as we transfer this color to shaders, alpha must be the last

    constexpr FColor grayscale() const
    {
        auto const y = 0.299F * r + 0.587F * g + 0.114F * b;
        return FColor {y, y, y, a};
    }

    constexpr auto operator<=>(FColor const&) const = default;
};

struct Color {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;

    constexpr FColor normalize() const
    {
        constexpr float max_value = std::numeric_limits<uint8_t>::max();
        return FColor(r / max_value, g / max_value, b / max_value, a / max_value);
    }

    constexpr Color grayscale() const
    {
        auto const [rn, gn, bn, _] = normalize();
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
