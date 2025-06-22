#pragma once

#include <any>
#include <array>
#include <format>
#include <functional>
#include <unordered_map>

#include "asset_type.h"
#include "shader_asset.h"

class IFileService;

namespace storm
{

class AssetServer final
{
public:
    struct NoCache;

    explicit AssetServer(IFileService const& file_service);
    ~AssetServer();

    template <typename Asset, typename... Args>
        requires(!std::is_pointer_v<Asset>)
    constexpr auto get_loader(std::string_view const& add_extension = {}) -> decltype(auto)
    {
        return [this, add_extension](std::filesystem::path const& path) -> decltype(auto) {
            constexpr auto asset_type     = static_cast<size_t>(asset_type_as_enum<std::remove_cvref_t<Asset>>());
            auto const&    asset_dir      = m_asset_dirs.at(asset_type);
            auto&          assets_by_type = m_assets.at(asset_type);

            if constexpr ((std::is_same_v<NoCache, Args> || ...)) {
                return load_asset<std::remove_cvref_t<Asset>>(asset_dir / path, add_extension);
            } else {
                if (!assets_by_type.contains(path.string())) {
                    assets_by_type.emplace(path.string(), load_asset<std::remove_cvref_t<Asset>>(asset_dir / path, add_extension));
                }

                return std::any_cast<Asset>(assets_by_type.at(path.string()));
            }
        };
    }

private:
    template <typename Asset>
        requires(!std::is_reference_v<Asset> && !std::is_const_v<Asset> && !std::is_volatile_v<Asset> && !std::is_pointer_v<Asset>)
    [[nodiscard]] static Asset load_asset(std::filesystem::path const& path, std::string_view const& add_extension)
    {
        auto file_path = path;
        if (!add_extension.empty()) { file_path.replace_extension(std::format("{}.{}", file_path.extension().string(), add_extension)); }

        auto const asset = asset_loader::from_file<Asset>(file_path);
        if (!asset.has_value()) { raise_loader_error(asset.error(), file_path); }

        return asset.value();
    }

    [[noreturn]] static void raise_loader_error(asset_loader::Error err, std::filesystem::path const& path);

    std::array<std::filesystem::path, ASSET_TYPE_COUNT> m_asset_dirs;

    std::array<std::unordered_map<std::string, std::any>, ASSET_TYPE_COUNT> m_assets;
};

}  // namespace storm
