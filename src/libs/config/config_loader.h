#pragma once

#include "i_config_loader.h"

namespace storm::config
{

class ConfigLoader: virtual public IConfigLoader
{
public:
    toml::value        open_config(std::filesystem::path const& path) override;
    toml::value const& open_config_cached(std::filesystem::path const& path) override;

private:
    std::unordered_map<std::filesystem::path, toml::value> m_files;
};

}  // namespace storm::config
