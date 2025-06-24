#include "shader_asset.h"

#include <fstream>

using namespace storm;

template <>
auto asset_loader::from_file<ShaderAsset>(std::filesystem::path const& path) -> std::expected<ShaderAsset, Error>
{
    auto shader_type = ShaderAssetType::Unknown;
    if (path.extension().string() == ".spv") {
        shader_type = ShaderAssetType::SPIRV;
    } else if (path.extension().string() == ".dxil") {
        shader_type = ShaderAssetType::DXIL;
    } else {
        return std::unexpected(Error::ExtensionNotSupported);
    }

    auto file = std::ifstream(path, std::ios::binary);

    file.seekg(0, std::ios::end);
    size_t const file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> shader_code;
    shader_code.resize(file_size);
    file.read(reinterpret_cast<char*>(shader_code.data()), file_size);

    return ShaderAsset {.path = path, .type = shader_type, .code = std::move(shader_code)};
}
