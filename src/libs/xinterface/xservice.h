#pragma once

#include <libs/renderer_next/cache.h>

#include "vx_service.h"
#include "xdefines.h"

namespace storm
{
class TextureAtlasNamed;
}

class XSERVICE: public VXSERVICE
{
    struct IMAGELISTDESCR {
        char*   sImageListName;
        char*   sTextureName;
        int32_t textureID;
        int     textureQuantity;

        int32_t textureWidth;
        int32_t textureHeight;
        int32_t pictureQuantity;
        int32_t pictureStart;

        std::unique_ptr<storm::TextureAtlasNamed> atlas;
    };

    struct PICTUREDESCR {
        char*  sPictureName;
        XYRECT pTextureRect;
    };

public:
    XSERVICE();
    ~XSERVICE() override;

    // initialization of service
    void Init(int32_t lWidth, int32_t lHight) override;

    void pre_draw_stage(storm::GPUCommandBuffer const& cmd_buffer, uint32_t delta_time) override;
    void update_stage(storm::GPUCopyPass const& copy_pass, uint32_t delta_time = 0) override;

    // get texture identificator for image group
    int32_t GetTextureID(char const* sImageListName) override;
    int32_t FindGroup(char const* sImageListName) const;
    bool    ReleaseTextureID(char const* sImageListName) override;

    auto get_texture(std::string_view const& image_list) -> std::shared_ptr<storm::GPUTexture> override;
    auto get_texture_uv(std::string_view const& image_list, std::string_view const& image) -> storm::FRect override;

    auto get_video_texture(std::string const& name) -> std::shared_ptr<storm::TextureSequence> override;

    // get texture positon for select picture
    bool GetTexturePos(int32_t pictureNum, FXYRECT& texRect) override;
    bool GetTexturePos(int32_t pictureNum, XYRECT& texRect) override;
    bool GetTexturePos(char const* sImageListName, char const* sImageName, FXYRECT& texRect) override;
    bool GetTexturePos(char const* sImageListName, char const* sImageName, XYRECT& texRect) override;
    bool GetTexturePos(int nTextureModify, int32_t pictureNum, FXYRECT& texRect) override;
    bool GetTexturePos(int nTextureModify, char const* sImageListName, char const* sImageName, FXYRECT& texRect) override;

    void GetTextureCutForSize(
        char const*     pcImageListName,
        const FXYPOINT& pntLeftTopUV,
        const XYPOINT&  pntSize,
        int32_t         nSrcWidth,
        int32_t         nSrcHeight,
        FXYRECT&        outUV) override;

    int32_t GetImageNum(char const* sImageListName, char const* sImageName) override;

    void ReleaseAll() override;

protected:
    void LoadAllPicturesInfo();

protected:
    storm::RendererCache m_cache;

    int32_t         m_dwListQuantity;
    int32_t         m_dwImageQuantity;
    IMAGELISTDESCR* m_pList;
    PICTUREDESCR*   m_pImage;

    // Scale factors
    float m_fWScale;
    float m_fHScale;
    // scaling error parameters
    float m_fWAdd;
    float m_fHAdd;
};

class ComboString
{
    struct STRING_DESCR {
        int   x, y;
        int   idFont;
        char* str;
    };

    struct PICS_DESCR {
        XYRECT  pos;
        int     idTex;
        FXYRECT texUV;
    };

    struct STRING_COMBINE {
        int* pFontsID;
        int  fontQ;

        STRING_DESCR* pStr;
        int           strQ;

        PICS_DESCR* pPics;
        int         picQ;
    };

    STRING_COMBINE* pComboStr;

    void AddToCombo(char* fontName, const XYPOINT& posStrStart, char* str);
    void AddToCombo(XYRECT posPic, char* picTexName, FXYRECT picUV);

public:
    ComboString();
    ~ComboString();

    void PrintComboString(int comboStrID);
    int  GetComboString(int align, int x, int y, int needWidth, int needHeight, int* allHeight, char* formatStr, char* fontlist);
    void ComboStringRelease(int comboStrID);
};
