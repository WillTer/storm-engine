#pragma once

#include <filesystem>
#include <unordered_map>

#include "i_config_loader.h"
#include "ini_file.h"

namespace storm
{

class ConfigLoader final: virtual public IConfigLoader
{
public:
    std::unique_ptr<IniFile> open_config(std::filesystem::path const& path) override;
    IniFile const&           open_config_cached(std::filesystem::path const& path) override;

private:
    std::unordered_map<std::filesystem::path, std::unique_ptr<IniFile>> m_files;
};

}  // namespace storm
