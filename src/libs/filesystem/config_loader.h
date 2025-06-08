#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include "config_file.hpp"
#include "i_config_loader.h"

namespace storm
{

class ConfigLoader final: virtual public IConfigLoader
{
public:
    std::shared_ptr<ConfigFile> open_config(std::filesystem::path const& path) override;
    std::shared_ptr<ConfigFile> open_config_cached(std::filesystem::path const& path) override;

private:
    std::unordered_map<std::filesystem::path, std::shared_ptr<ConfigFile>> m_files;
};

}  // namespace storm
