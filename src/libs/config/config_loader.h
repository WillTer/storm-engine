#pragma once

#include <filesystem>
#include <unordered_map>

#include "i_config_loader.h"

namespace storm
{

class ConfigLoader final: virtual public IConfigLoader
{
public:
    toml::value open_config(std::filesystem::path const& path) override;
    toml::value open_config_cached(std::filesystem::path const& path) override;

private:
    std::unordered_map<std::filesystem::path, toml::value> m_files;
};

}  // namespace storm
