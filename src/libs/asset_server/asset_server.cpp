#include "asset_server.h"

#include <format>
#include <stdexcept>

#include <libs/core/core.h>

#include "shader_asset.h"
#include "texture_asset.h"

using namespace storm;

AssetServer::AssetServer(std::shared_ptr<IFileService> const& file_service) : m_file_service(file_service)
{
    m_asset_dirs[TextureAsset::type_name()]  = file_service->base_directory_path(BaseDirectory::Textures);
    m_asset_dirs[TextFileAsset::type_name()] = file_service->base_directory_path(BaseDirectory::Ini);
    // Load shaders from path relative to executable instead of working directory
    // This helps in developing engine as you do not need to copy compiled shaders to game directory
    m_asset_dirs[ShaderAsset::type_name()] =
        file_service->executable_directory() / file_service->base_directory_path(BaseDirectory::Shaders);
}

AssetServer::~AssetServer() = default;

TextureAsset AssetServer::load_texture_file(std::filesystem::path const& path, bool const use_asset_dir /*= true*/)
{
    static auto loader = get_loader<TextureAsset>();
    return loader(path, use_asset_dir);
}

ShaderAsset AssetServer::load_shader_file(std::filesystem::path const& path, bool const use_asset_dir /*= true*/)
{
    static auto loader = get_loader<ShaderAsset>();
    return loader(path, use_asset_dir);
}

TextFileAsset AssetServer::load_text_file(std::filesystem::path const& path, bool const use_asset_dir /*= true*/)
{
    static auto loader = get_loader<TextFileAsset>();
    return loader(path, use_asset_dir);
}

[[noreturn]] void AssetServer::raise_loader_error(asset_loader::Error const err, std::filesystem::path const& path)
{
    switch (err) {
    case asset_loader::Error::FileNotFound: throw std::runtime_error(std::format("File not found: {}", path.string()));
    case asset_loader::Error::ExtensionNotSupported:
        throw std::runtime_error(std::format("File extension is not supported for selected asset type: {}", path.string()));
    case asset_loader::Error::NotImplemented:
        throw std::runtime_error(std::format("Loader not implemented for selected asset: {}", path.string()));
        break;
    }

    throw std::runtime_error(std::format("Failed to load asset: {}", path.string()));
}
