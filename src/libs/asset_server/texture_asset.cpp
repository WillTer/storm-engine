#include "texture_asset.h"

#include <cstring>

using namespace storm;

template <>
auto asset_loader::from_file<TextureAsset>(std::filesystem::path const& path) -> std::expected<TextureAsset, Error>
{
    if (path.extension().string() != ".tx") { return std::unexpected(Error::ExtensionNotSupported); }

    auto const content = from_file<std::vector<char>>(path).value();

    TxFileHeader header = {};
    std::memcpy(&header, content.data(), sizeof(header));

    std::vector<char> data(content.size() - sizeof(header));
    std::memcpy(data.data(), content.data() + sizeof(header), data.size());

    auto asset        = TextureAsset {};
    asset.path        = path.string();
    asset.path_hashed = entt::hashed_string(asset.path.c_str());
    asset.header      = header;
    asset.data        = std::move(data);

    return asset;
}
