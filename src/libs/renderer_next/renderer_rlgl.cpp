#include "renderer_rlgl.h"

#include <format>
#include <memory>
#include <stdexcept>

#include <libs/asset_server/asset_server.h>
#include <libs/asset_server/shader_asset.h>
#include <libs/asset_server/texture_asset.h>
#include <libs/core/core.h>

using namespace storm;

namespace
{
#ifdef _DEBUG
constexpr bool IS_DEBUG_MODE = true;
#else
constexpr bool IS_DEBUG_MODE = false;
#endif

// TODO: move to another file
PixelFormat convert_tx_format(TxFormat const format)
{
    switch (format) {
    case TxFormat::A8R8G8B8: return PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    case TxFormat::X8R8G8B8: return PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    case TxFormat::R5G6B5: return PIXELFORMAT_UNCOMPRESSED_R5G6B5;
    case TxFormat::A1R5G5B5: return PIXELFORMAT_UNCOMPRESSED_R5G5B5A1;
    case TxFormat::A4R4G4B4: return PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;
    case TxFormat::L8: return PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

    case TxFormat::DXT1: return PIXELFORMAT_COMPRESSED_DXT1_RGBA;
    case TxFormat::DXT2: return PIXELFORMAT_COMPRESSED_DXT3_RGBA;
    case TxFormat::DXT3: return PIXELFORMAT_COMPRESSED_DXT3_RGBA;
    case TxFormat::DXT4: return PIXELFORMAT_COMPRESSED_DXT5_RGBA;
    case TxFormat::DXT5: return PIXELFORMAT_COMPRESSED_DXT5_RGBA;
    default: break;
    }

    return PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
}

}  // namespace

RendererRlgl::RendererRlgl() {}

RendererRlgl::~RendererRlgl() = default;

void RendererRlgl::init()
{
    auto const& asset_server = core->get<AssetServer>();
    auto const  shader_load  = asset_server->get_loader<ShaderAsset, AssetServer::NoCache>("glsl");

    // Testing
    auto  texture = asset_server->get_texture("loading/storm.tga.tx");
    Image image   = {
          .data    = reinterpret_cast<void*>(texture.data.data()),
          .width   = static_cast<int>(texture.header.width),
          .height  = static_cast<int>(texture.header.height),
          .mipmaps = static_cast<int>(texture.header.mip_levels),
          .format  = convert_tx_format(texture.header.format),
    };

    m_loading = LoadTextureFromImage(image);
}

void RendererRlgl::draw()
{
    BeginDrawing();
    DrawTexture(m_loading, 0, 0, WHITE);
    EndDrawing();
}
