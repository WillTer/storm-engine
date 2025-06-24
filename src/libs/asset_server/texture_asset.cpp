#include "texture_asset.h"

#include <fstream>

using namespace storm;

template <>
auto asset_loader::from_file<TextureAsset>(std::filesystem::path const& path) -> std::expected<TextureAsset, Error>
{
    if (path.extension().string() != ".tx") { return std::unexpected(Error::ExtensionNotSupported); }

    auto         file   = std::ifstream(path, std::ios::binary);
    TxFileHeader header = {};
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    auto cur_pos = file.tellg();
    file.seekg(0, std::ios::end);
    size_t const data_size = file.tellg() - cur_pos;
    file.seekg(cur_pos, std::ios::beg);

    std::vector<uint8_t> data(data_size);
    file.read(reinterpret_cast<char*>(data.data()), data_size);

    return TextureAsset {.path = path, .header = header, .data = data};
}
