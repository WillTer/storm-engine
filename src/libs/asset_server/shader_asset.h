#pragma once

#include <vector>

#include <entt/core/hashed_string.hpp>

#include "asset_loader.h"

namespace storm
{

enum class ShaderAssetType {
    Unknown,
    SPIRV,
    DXIL,
};

struct ShaderAsset {
    std::filesystem::path path;
    ShaderAssetType       type;
    std::vector<char>     code;

    constexpr static entt::hashed_string type_name()
    {
        constexpr static entt::hashed_string type_name = "ShaderAsset";
        return type_name;
    }
};

namespace asset_loader
{

template <>
std::expected<ShaderAsset, Error> from_file<ShaderAsset>(std::filesystem::path const& path);

}  // namespace asset_loader

}  // namespace storm
