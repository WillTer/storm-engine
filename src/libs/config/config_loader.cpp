#include "config_loader.h"

#include <libs/core/core.h>
#include <libs/core/v_file_service.h>

using namespace storm::config;

toml::value ConfigLoader::open_config(std::filesystem::path const& path)
{
    if (!fio->is_path_exists(path)) {
        core.Trace("File not found: %s", path.string().c_str());
        return {};
    }

    auto stream = fio->open_file<std::ifstream>(path, std::ios::binary);

    try {
        return toml::parse(stream, path.string());
    } catch (toml::exception const& e) {
        core.Trace("%s", e.what());
    }

    return {};
}

toml::value const& ConfigLoader::open_config_cached(std::filesystem::path const& path)
{
    // Add to cache even if load fails
    // Then try to parse file again when someone asks it
    if (!m_files.contains(path) || m_files.at(path).is_empty()) { m_files[path] = open_config(path); }

    return m_files.at(path);
}
