#pragma once

#include <vector>

#include "asset_loader.h"
#include "asset_type.h"

namespace storm
{

struct ShaderAsset {
    std::vector<uint8_t> code;
};

template <>
constexpr AssetType asset_type_as_enum<ShaderAsset>()
{
    return AssetType::Shader;
}

namespace asset_loader
{

template <>
std::expected<ShaderAsset, Error> from_file<ShaderAsset>(std::filesystem::path const& path);

}  // namespace asset_loader

}  // namespace storm
