#pragma once

#include <type_traits>

namespace storm::type_traits
{

template <typename T, bool = std::is_enum_v<T>>
struct is_flag;

template <typename T>
struct is_flag<T, true>: std::false_type {
};

}  // namespace storm::type_traits

// Bit operators for flag types
template <typename T>
    requires storm::type_traits::is_flag<T>::value
inline T operator|(T l, T r)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(l) | static_cast<std::underlying_type_t<T>>(r));
}

template <typename T>
    requires storm::type_traits::is_flag<T>::value
inline T operator&(T l, T r)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(l) & static_cast<std::underlying_type_t<T>>(r));
}

template <typename T>
    requires storm::type_traits::is_flag<T>::value
inline T operator^(T l, T r)
{
    return static_cast<T>(static_cast<std::underlying_type_t<T>>(l) ^ static_cast<std::underlying_type_t<T>>(r));
}

template <typename T>
    requires storm::type_traits::is_flag<T>::value
inline T operator~(T l)
{
    return static_cast<T>(~static_cast<std::underlying_type_t<T>>(l));
}

template <typename T>
    requires storm::type_traits::is_flag<T>::value
inline bool is_flag_enabled(T value, T flag)
{
    return (value & flag) == flag;
}
