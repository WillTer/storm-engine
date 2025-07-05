#pragma once

#include <vector>

#include <entt/core/hashed_string.hpp>

#include "asset_loader.h"

namespace storm
{

enum class TxFormat {
    A8R8G8B8 = 21,
    X8R8G8B8 = 22,
    R5G6B5   = 23,
    A1R5G5B5 = 25,
    A4R4G4B4 = 26,
    P8       = 41,  // Unsupported
    L8       = 50,
    V8U8     = 60,
    L6V5U5   = 61,         // Unsupported
    DXT1     = 827611204,  // '1TXD',
    DXT2     = 844388420,  // '2TXD',
    DXT3     = 861165636,  // '3TXD',
    DXT4     = 877942852,  // '4TXD',
    DXT5     = 894720068,  // '5TXD',
};

enum TxFlags {
    TX_FLAGS_NONE     = 0,
    TX_FLAGS_PALLETTE = 1,
    TX_FLAGS_CUBEMAP  = 2,
};

// File header
struct TxFileHeader {
    uint32_t flags;  // TxFlags
    uint32_t width;
    uint32_t height;
    uint32_t mip_levels;  // number of mip levels
    TxFormat format;      // texture format
    uint32_t mip_size;    // size of mip 0 (width*height*pixel_size)
};

struct TextureAsset {
    std::string         path;
    entt::hashed_string path_hashed;
    TxFileHeader        header;
    std::vector<char>   data;
};

namespace asset_loader
{

template <>
std::expected<TextureAsset, Error> from_file<TextureAsset>(std::filesystem::path const& path);

}  // namespace asset_loader

}  // namespace storm
