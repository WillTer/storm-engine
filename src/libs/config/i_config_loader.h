#pragma once

#include <filesystem>

#include <toml_fwd.hpp>

namespace storm
{

class IConfigLoader
{
public:
    virtual ~IConfigLoader() = default;

    virtual toml::value open_config(std::filesystem::path const& path)        = 0;
    virtual toml::value open_config_cached(std::filesystem::path const& path) = 0;
};

}  // namespace storm

using ConfigLoaderPtr = std::shared_ptr<storm::IConfigLoader>;
