#pragma once

#include <vector>

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
};

namespace asset_loader
{

template <>
std::expected<ShaderAsset, Error> from_file<ShaderAsset>(std::filesystem::path const& path);

}  // namespace asset_loader

}  // namespace storm
