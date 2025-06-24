#include "shader_asset.h"

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

    return ShaderAsset {.path = path, .type = shader_type, .code = from_file<std::vector<char>>(path).value()};
}
