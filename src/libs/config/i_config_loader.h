#pragma once

#include <filesystem>
#include <memory>

#include "config_file.hpp"

namespace storm
{

class IConfigLoader
{
public:
    virtual ~IConfigLoader() = default;

    virtual std::shared_ptr<ConfigFile> open_config(std::filesystem::path const& path)        = 0;
    virtual std::shared_ptr<ConfigFile> open_config_cached(std::filesystem::path const& path) = 0;
};

}  // namespace storm
