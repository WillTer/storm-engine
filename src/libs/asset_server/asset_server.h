#pragma once

#include <any>
#include <format>
#include <functional>
#include <unordered_map>

#include <entt/core/type_info.hpp>
#include <libs/filesystem/v_file_service.h>
#include <libs/util/hashed_string_map.h>

#include "shader_asset.h"
#include "text_file_asset.h"
#include "texture_asset.h"

namespace storm
{

class AssetServer final
{
public:
    struct UseCache;

    explicit AssetServer(std::shared_ptr<IFileService> const& file_service);
    ~AssetServer();

    template <typename Asset, typename... Args>
        requires(!std::is_pointer_v<Asset>)
    constexpr auto get_loader(std::string_view const& add_extension = {}) -> decltype(auto)
    {
        return [this, add_extension](std::filesystem::path const& path, bool const use_asset_dir = true) -> decltype(auto) {
            auto const asset_type = entt::type_id<Asset>().index();
            auto const file_path  = use_asset_dir && m_asset_dirs.contains(asset_type) ? m_asset_dirs.at(asset_type) / path : path;

            if (!m_assets.contains(asset_type)) { m_assets.emplace(asset_type, std::unordered_map<std::string, std::any> {}); }
            auto& assets_by_type = m_assets.at(asset_type);

            auto const& ext = add_extension.empty() && m_asset_ext.contains(asset_type) ? m_asset_ext.at(asset_type) : add_extension;

            if constexpr ((std::is_same_v<UseCache, Args> || ...)) {
                if (!assets_by_type.contains(file_path.string())) {
                    assets_by_type.emplace(file_path.string(), load_asset<std::remove_cvref_t<Asset>>(*m_file_service, file_path, ext));
                }

                return std::any_cast<Asset>(assets_by_type.at(file_path.string()));
            } else {
                return load_asset<std::remove_cvref_t<Asset>>(*m_file_service, file_path, ext);
            }
        };
    }

    template <typename Asset>
    void set_asset_dir(std::filesystem::path const& asset_dir)
    {
        m_asset_dirs.emplace(entt::type_id<Asset>().index(), asset_dir);
    }

    template <typename Asset>
    void set_asset_dir(BaseDirectory const base_dir)
    {
        auto const asset_dir = m_file_service->base_directory_path(base_dir);
        m_asset_dirs.emplace(entt::type_id<Asset>().index(), asset_dir);
    }

    template <typename Asset>
    void set_asset_ext(std::string const& extension)
    {
        m_asset_ext.emplace(entt::type_id<Asset>().index(), extension);
    }

    TextureAsset  load_texture_file(std::filesystem::path const& path, bool use_asset_dir = true);
    ShaderAsset   load_shader_file(std::filesystem::path const& path, bool use_asset_dir = true);
    TextFileAsset load_text_file(std::filesystem::path const& path, bool use_asset_dir = true);

private:
    template <typename Asset>
        requires(!std::is_reference_v<Asset> && !std::is_const_v<Asset> && !std::is_volatile_v<Asset> && !std::is_pointer_v<Asset>)
    [[nodiscard]] static Asset
    load_asset(IFileService const& file_service, std::filesystem::path const& path, std::string_view const& add_extension)
    {
        auto file_path = file_service.transform_path(path);
        if (!add_extension.empty()) { file_path.replace_extension(std::format("{}.{}", file_path.extension().string(), add_extension)); }

        if (!std::filesystem::exists(file_path)) { raise_loader_error(asset_loader::Error::FileNotFound, file_path); }

        auto const asset = asset_loader::from_file<Asset>(file_path);
        if (!asset.has_value()) { raise_loader_error(asset.error(), file_path); }

        return asset.value();
    }

    [[noreturn]] static void raise_loader_error(asset_loader::Error err, std::filesystem::path const& path);

    std::shared_ptr<IFileService> m_file_service;

    id_type_map<std::filesystem::path>                     m_asset_dirs;
    id_type_map<std::string>                               m_asset_ext;
    id_type_map<std::unordered_map<std::string, std::any>> m_assets;
};

}  // namespace storm
