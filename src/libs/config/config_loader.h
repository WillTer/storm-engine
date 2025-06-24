#pragma once

#include <filesystem>
#include <unordered_map>

#include "i_config_loader.h"
#include "ini_file.h"

namespace storm
{

class AssetServer;
class ConfigLoader final: virtual public IConfigLoader
{
public:
    explicit ConfigLoader(std::shared_ptr<AssetServer> const& asset_server);

    std::unique_ptr<IniFile> open_config(std::filesystem::path const& path, bool search_in_config_dir = true) override;
    IniFile const&           open_config_cached(std::filesystem::path const& path, bool search_in_config_dir = true) override;

private:
    std::shared_ptr<AssetServer>                                        m_asset_server;
    std::unordered_map<std::filesystem::path, std::unique_ptr<IniFile>> m_files;
};

}  // namespace storm
