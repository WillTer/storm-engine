#include "texture_asset.h"

#include <fstream>

#include <libs/core/core.h>

using namespace storm;

template <>
std::expected<TextureAsset, asset_loader::Error> asset_loader::from_file<TextureAsset>(std::filesystem::path const& path)
{
    auto const& file_service = core->get<IFileService>();

    if (!file_service->exists(path)) { return std::unexpected(Error::FileNotFound); }

    if (path.extension().string() != ".tx") { return std::unexpected(Error::ExtensionNotSupported); }

    auto         file   = file_service->open_file<std::ifstream>(path, std::ios::binary);
    TxFileHeader header = {};
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    auto cur_pos = file.tellg();
    file.seekg(0, std::ios::end);
    size_t const data_size = file.tellg() - cur_pos;
    file.seekg(cur_pos, std::ios::beg);

    std::vector<uint8_t> data(data_size);
    file.read(reinterpret_cast<char*>(data.data()), data_size);

    return TextureAsset {.header = header, .data = data};
}
