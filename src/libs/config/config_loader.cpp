#include "config_loader.h"

#include <libs/asset_server/asset_server.h>
#include <spdlog/spdlog.h>

#include "ini_file.h"

using namespace storm;

ConfigLoader::ConfigLoader(std::shared_ptr<AssetServer> const& asset_server) : m_asset_server(asset_server)
{
    assert(m_asset_server);
}

std::unique_ptr<IniFile> ConfigLoader::open_config(std::filesystem::path const& path, bool const search_in_config_dir /*= true*/)
{
    auto const text_file = m_asset_server->load_text_file(path, search_in_config_dir);

    if (text_file.type != TextFileAssetType::Ini) {
        spdlog::error("Config file \"%s\" was not loaded - extension is not supported", path.string());
        return {};
    }

    return std::make_unique<IniFile>(text_file);
}

IniFile const& ConfigLoader::open_config_cached(std::filesystem::path const& path, bool const search_in_config_dir /*= true*/)
{
    if (!m_files.contains(path)) {
        auto config = open_config(path, search_in_config_dir);
        if (!config) { throw std::runtime_error("Could not open config file"); }

        m_files.emplace(path, std::move(config));
    }

    return *m_files.at(path);
}
