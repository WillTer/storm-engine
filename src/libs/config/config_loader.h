#pragma once

#include <filesystem>
#include <memory>
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

    std::shared_ptr<ConfigFile> open_config(std::filesystem::path const& path) override;
    std::shared_ptr<ConfigFile> open_config_cached(std::filesystem::path const& path) override;

private:
    std::unordered_map<std::filesystem::path, std::shared_ptr<ConfigFile>> m_files;

    IFileService& m_fs;
};

}  // namespace storm
