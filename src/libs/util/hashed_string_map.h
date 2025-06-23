#pragma once

#include <unordered_map>

#include <entt/core/hashed_string.hpp>

namespace storm
{

struct hashed_string_map_hash {
    constexpr std::size_t operator()(entt::hashed_string const& key) const
    {
        return key.value();
    }
};

struct id_type_map_hash {
    constexpr std::size_t operator()(entt::id_type const& key) const
    {
        return key;
    }
};

template <typename T>
using hashed_string_map = std::unordered_map<entt::hashed_string, T, hashed_string_map_hash>;

template <typename T>
using id_type_map = std::unordered_map<entt::id_type, T, id_type_map_hash>;

}  // namespace storm
