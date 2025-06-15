#include "config_loader.h"

#include <libs/core/core.h>
#include <libs/filesystem/v_file_service.h>

#include "ini_file.h"

using namespace storm;

std::unique_ptr<IniFile> ConfigLoader::open_config(std::filesystem::path const& path)
{
    if (!fio->exists(path)) {
        core->Trace("Config file \"%s\" not found", fio->transform_path(path).string().c_str());
        return {};
    }

    if (path.extension().string() != ".ini") {
        core->Trace("Config file \"%s\" was not loaded - extension is not supported", path.string().c_str());
        return {};
    }

    return std::make_unique<IniFile>(path);
}

IniFile const& ConfigLoader::open_config_cached(std::filesystem::path const& path)
{
    // Add to cache even if load fails
    // Then try to parse file again when someone asks it
    if (!m_files.contains(path) || m_files.at(path)->is_empty()) { m_files[path] = open_config(path); }

    return *m_files.at(path);
}
