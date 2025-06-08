#include "config_loader.h"

#include <libs/core/core.h>

#include "config_file_toml.h"
#include "v_file_service.h"

using namespace storm;

std::shared_ptr<ConfigFile> ConfigLoader::open_config(std::filesystem::path const& path)
{
    if (!fio->exists(path)) {
        core.Trace("Config file \"%s\" not found", path.string().c_str());
        return nullptr;
    }

    if (path.extension().string() == ".toml") { return std::make_shared<ConfigFileToml>(path); }
    core.Trace("Config file extension \"%s\" is unknown", path.extension().string().c_str());

    return nullptr;
}

std::shared_ptr<ConfigFile> ConfigLoader::open_config_cached(std::filesystem::path const& path)
{
    // Add to cache even if load fails
    // Then try to parse file again when someone asks it
    if (!m_files.contains(path) || !m_files.at(path)) { m_files[path] = open_config(path); }

    return m_files.at(path);
}
