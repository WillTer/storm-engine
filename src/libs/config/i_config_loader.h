#pragma once

#include <filesystem>

namespace storm
{

class IniFile;

class IConfigLoader
{
public:
    virtual ~IConfigLoader() = default;

    virtual std::unique_ptr<IniFile> open_config(std::filesystem::path const& path)        = 0;
    virtual IniFile const&           open_config_cached(std::filesystem::path const& path) = 0;
};

}  // namespace storm

extern std::shared_ptr<storm::IConfigLoader> config_loader;
