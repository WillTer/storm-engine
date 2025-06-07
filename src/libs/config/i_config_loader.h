#pragma once

#include <filesystem>

#include <toml.hpp>

namespace storm::config
{

class IConfigLoader
{
public:
    virtual ~IConfigLoader() = default;

    virtual toml::value        open_config(std::filesystem::path const& path)        = 0;
    virtual toml::value const& open_config_cached(std::filesystem::path const& path) = 0;
};

}  // namespace storm::config
