#pragma once

#include <vector>

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
};

namespace asset_loader
{

template <>
std::expected<TextFileAsset, Error> from_file<TextFileAsset>(std::filesystem::path const& path);

}  // namespace asset_loader

}  // namespace storm
