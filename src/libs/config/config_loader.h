#pragma once

#include <filesystem>
#include <unordered_map>

#include <libs/filesystem/v_file_service.h>

#include "i_config_loader.h"

namespace storm
{

class ConfigLoader final: virtual public IConfigLoader
{
public:
    // TODO: use shared/weak ptr for file_service
    explicit ConfigLoader(IFileService& file_service);

    toml::value open_config(std::filesystem::path const& path) override;
    toml::value open_config_cached(std::filesystem::path const& path) override;

private:
    std::unordered_map<std::filesystem::path, toml::value> m_files;

    IFileService& m_fs;
};

}  // namespace storm
