#pragma once

#include <any>
#include <format>
#include <functional>
#include <unordered_map>

#include <libs/util/hashed_string_map.h>

#include "shader_asset.h"
#include "texture_asset.h"

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
            constexpr auto asset_type = std::remove_cvref_t<Asset>::type_name();
            auto const&    asset_dir  = m_asset_dirs.contains(asset_type) ? m_asset_dirs.at(asset_type) : m_default_dir;

            if (!m_assets.contains(asset_type)) { m_assets.emplace(asset_type, std::unordered_map<std::string, std::any> {}); }
            auto& assets_by_type = m_assets.at(asset_type);

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

    template <typename Asset>
    void set_asset_dir(std::filesystem::path const& asset_dir)
    {
        m_asset_dirs.emplace(std::remove_cvref_t<Asset>::type_name(), asset_dir);
    }

    TextureAsset const& get_texture(std::filesystem::path const& path);

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

    hashed_string_map<std::filesystem::path>                     m_asset_dirs;
    hashed_string_map<std::unordered_map<std::string, std::any>> m_assets;

    std::filesystem::path m_default_dir;
};

}  // namespace storm
