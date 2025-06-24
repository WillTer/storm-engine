#pragma once

#include <vector>

#include <entt/core/hashed_string.hpp>

#include "asset_loader.h"

namespace storm
{

enum class TextFileAssetType {
    Unknown,
    Ini,
};

struct TextFileAsset {
    std::filesystem::path path;
    TextFileAssetType     type;
    std::vector<char>     content;

    constexpr static entt::hashed_string type_name()
    {
        constexpr static entt::hashed_string type_name = "TextFileAsset";
        return type_name;
    }
};

namespace asset_loader
{

template <>
std::expected<TextFileAsset, Error> from_file<TextFileAsset>(std::filesystem::path const& path);

}  // namespace asset_loader

}  // namespace storm
