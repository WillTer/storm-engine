#pragma once

namespace storm
{

enum class AssetType {
    Unknown = 0,
    Shader,
    Texture,

    Last,
};

constexpr size_t ASSET_TYPE_COUNT = static_cast<size_t>(AssetType::Last);

template <typename T>
constexpr AssetType asset_type_as_enum()
{
    return AssetType::Unknown;
}

}  // namespace storm
