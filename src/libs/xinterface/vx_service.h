#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include <libs/renderer_next/types.h>

#define TEXTURE_MODIFY_NONE 0
#define TEXTURE_MODIFY_HORZFLIP 1
#define TEXTURE_MODIFY_VERTFLIP 2

struct FXYRECT;
struct XYRECT;
struct FXYPOINT;
struct XYPOINT;

namespace storm
{
class GPUCopyPass;
class GPUTexture;
}  // namespace storm

class VXSERVICE
{
public:
    virtual ~VXSERVICE()                                                                   = default;
    virtual void Init(storm::GPUCopyPass const& copy_pass, int32_t lWidth, int32_t lHight) = 0;

    // get texture identificator for image group
    virtual int32_t GetTextureID(char const* sImageListName)     = 0;
    virtual bool    ReleaseTextureID(char const* sImageListName) = 0;

    virtual auto get_texture(std::string_view const& image_list) -> std::shared_ptr<storm::GPUTexture>             = 0;
    virtual auto get_texture_uv(std::string_view const& image_list, std::string_view const& image) -> storm::FRect = 0;

    // get texture positon for select picture
    virtual bool GetTexturePos(int32_t pictureNum, FXYRECT& texRect)                                                     = 0;
    virtual bool GetTexturePos(int32_t pictureNum, XYRECT& texRect)                                                      = 0;
    virtual bool GetTexturePos(char const* sImageListName, char const* sImageName, FXYRECT& texRect)                     = 0;
    virtual bool GetTexturePos(char const* sImageListName, char const* sImageName, XYRECT& texRect)                      = 0;
    virtual bool GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT& texRect)                                 = 0;
    virtual bool GetTexturePos(int nTextureModify, char const* sImageListName, char const* sImageName, FXYRECT& texRect) = 0;

    virtual void GetTextureCutForSize(
        char const*     pcImageListName,
        const FXYPOINT& pntLeftTopUV,
        const XYPOINT&  pntSize,
        int32_t         nSrcWidth,
        int32_t         nSrcHeight,
        FXYRECT&        outUV) = 0;

    virtual int32_t GetImageNum(char const* sImageListName, char const* sImageName) = 0;

    virtual void ReleaseAll() = 0;
};
