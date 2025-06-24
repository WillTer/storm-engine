#include "text_file_asset.h"

#include <fstream>

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

    auto file = std::ifstream(path, std::ios::binary);  // binary for UTF-8 loading

    file.seekg(0, std::ios::end);
    size_t const file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> content;
    content.resize(file_size);
    file.read(content.data(), file_size);

    return TextFileAsset {.path = path, .type = file_type, .content = std::move(content)};
}
