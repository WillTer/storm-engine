#include "shader_asset.h"

#include <fstream>

#include <libs/core/core.h>

using namespace storm;

template <>
std::expected<ShaderAsset, asset_loader::Error> asset_loader::from_file<ShaderAsset>(std::filesystem::path const& path)
{
    auto const& file_service = core->get<IFileService>();

    if (!file_service->exists(path)) { return std::unexpected(Error::FileNotFound); }

    auto shader_type = ShaderAssetType::Unknown;
    if (path.extension().string() == ".spv") {
        shader_type = ShaderAssetType::SPIRV;
    } else if (path.extension().string() == ".bin") {
        shader_type = ShaderAssetType::DXIL;
    } else {
        return std::unexpected(Error::ExtensionNotSupported);
    }

    auto file = file_service->open_file<std::ifstream>(path, std::ios::binary);

    file.seekg(0, std::ios::end);
    size_t const file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> shader_code;
    shader_code.resize(file_size);
    file.read(reinterpret_cast<char*>(shader_code.data()), file_size);

    return ShaderAsset {.type = shader_type, .code = std::move(shader_code)};
}
