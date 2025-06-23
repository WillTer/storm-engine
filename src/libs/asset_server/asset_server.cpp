#include "asset_server.h"

#include <format>
#include <stdexcept>

#include <libs/filesystem/v_file_service.h>

#include "shader_asset.h"
#include "texture_asset.h"

using namespace storm;

AssetServer::AssetServer(IFileService const& file_service)
{
    m_default_dir = file_service.base_directory_path(BaseDirectory::Resource);

    m_asset_dirs[TextureAsset::type_name()] = file_service.base_directory_path(BaseDirectory::Textures);
    // Load shaders from path relative to executable instead of working directory
    // This helps in developing engine as you do not need to copy compiled shaders to game directory
    m_asset_dirs[ShaderAsset::type_name()] = file_service.executable_directory() / file_service.base_directory_path(BaseDirectory::Shaders);
}

AssetServer::~AssetServer() = default;

TextureAsset const& AssetServer::get_texture(std::filesystem::path const& path)
{
    static auto loader = get_loader<TextureAsset const&>();
    return loader(path);
}

[[noreturn]] void AssetServer::raise_loader_error(asset_loader::Error const err, std::filesystem::path const& path)
{
    switch (err) {
    case asset_loader::Error::FileNotFound: throw std::runtime_error(std::format("File not found: {}", path.string()));
    case asset_loader::Error::ExtensionNotSupported:
        throw std::runtime_error(std::format("File extension is not supported for selected asset type: {}", path.string()));
    }
}
