#include "text_file_asset.h"

using namespace storm;

template <>
auto asset_loader::from_file<TextFileAsset>(std::filesystem::path const& path) -> std::expected<TextFileAsset, Error>
{
    auto file_type = TextFileAssetType::Unknown;
    if (path.extension().string() == ".ini") {
        file_type = TextFileAssetType::Ini;
    } else {
        return std::unexpected(Error::ExtensionNotSupported);
    }

    return TextFileAsset {.path = path, .type = file_type, .content = from_file<std::vector<char>>(path).value()};
}
