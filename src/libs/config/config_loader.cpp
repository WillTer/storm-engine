#include "config_loader.h"

#include <libs/core/core.h>
#include <libs/filesystem/v_file_service.h>
#include <toml.hpp>

using namespace storm;

ConfigLoader::ConfigLoader(IFileService& file_service) : m_fs {file_service} {}

toml::value ConfigLoader::open_config(std::filesystem::path const& path)
{
    if (!m_fs.exists(path)) {
        core.Trace("Config file \"%s\" not found", path.string().c_str());
        return {};
    }

    if (path.extension().string() != ".toml") {
        core.Trace("Config file \"%s\" was not loaded - extension is not supported", path.string().c_str());
        return {};
    }

    auto file_stream = m_fs.open_file<std::ifstream>(path, std::ios::binary);
    return toml::parse(file_stream);
}

toml::value ConfigLoader::open_config_cached(std::filesystem::path const& path)
{
    // Add to cache even if load fails
    // Then try to parse file again when someone asks it
    if (!m_files.contains(path) || m_files.at(path).is_empty()) { m_files[path] = open_config(path); }

    return m_files.at(path);
}
