#pragma once

#include <filesystem>

#include "config_file.hpp"

namespace storm
{

class ConfigFileToml final: public ConfigFile
{
public:
    explicit ConfigFileToml(std::filesystem::path const& path);
};

}  // namespace storm