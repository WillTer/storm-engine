#pragma once

#include <filesystem>

namespace storm
{

class IniFile;

class IConfigLoader
{
public:
    virtual ~IConfigLoader() = default;

    virtual std::unique_ptr<IniFile> open_config(std::filesystem::path const& path, bool search_in_config_dir = true)        = 0;
    virtual IniFile const&           open_config_cached(std::filesystem::path const& path, bool search_in_config_dir = true) = 0;
};

}  // namespace storm
