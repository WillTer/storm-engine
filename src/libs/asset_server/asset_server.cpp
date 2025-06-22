#include "asset_server.h"

#include <format>
#include <stdexcept>

#include <libs/filesystem/v_file_service.h>

using namespace storm;

AssetServer::AssetServer(IFileService const& file_service)
{
    m_asset_dirs[static_cast<size_t>(AssetType::Unknown)] = file_service.base_directory_path(BaseDirectory::Resource);
    // Load shaders from path relative to executable instead of working directory
    // This helps in developing engine as you do not need to copy compiled shaders to game directory
    m_asset_dirs[static_cast<size_t>(AssetType::Shader)] =
        file_service.executable_directory() / file_service.base_directory_path(BaseDirectory::Shaders);
}

AssetServer::~AssetServer() = default;

[[noreturn]] void AssetServer::raise_loader_error(asset_loader::Error const err, std::filesystem::path const& path)
{
    switch (err) {
    case asset_loader::Error::FileNotFound: throw std::runtime_error(std::format("File not found: {}", path.string()));
    case asset_loader::Error::ExtensionNotSupported:
        throw std::runtime_error(std::format("File extension is not supported for selected asset type: {}", path.string()));
    }

    throw std::runtime_error(std::format("Failed to load asset: {}", path.string()));
}
