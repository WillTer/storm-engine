#pragma once

#include <filesystem>

#include <libs/filesystem/v_file_service.h>

#include "config_file.hpp"


namespace storm
{

class ConfigFileToml final: public ConfigFile
{
public:
    explicit ConfigFileToml(IFileService& file_service, std::filesystem::path const& path);
};

}  // namespace storm
