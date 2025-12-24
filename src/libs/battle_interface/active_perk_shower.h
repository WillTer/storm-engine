#pragma once

#include <cstdint>

#include <libs/core/entity.h>
#include <libs/renderer_next/types.h>

class ActivePerkShower: public Entity
{
    // FIXME: Renderer Next
    // VDX9RENDER* rs;

public:
    ActivePerkShower();
    ~ActivePerkShower() override;
    bool     Init() override;
    void     Execute(uint32_t delta_time);
    void     Realize(uint32_t delta_time) const;
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
              LostRender(delta); break;
            case Stage::restore_render:
              RestoreRender(delta); break;*/
        }
    }

protected:
    void ReleaseAll();

    int32_t m_idVBuf;
    int32_t m_idIBuf;
    void    FillVIBuffers();
    void    FillRectData(void* vbuf, storm::FRect const& rectPos, storm::FRect const& rectTex);

    size_t m_nTextureQ;

    struct _TEXTURE_DESCR {
        int32_t m_idTexture;
        int32_t m_nCol;
        int32_t m_nRow;

        int32_t m_nPicsQ;
        int32_t m_nVertStart;
        int32_t m_nIndxStart;
    }* m_pTexDescr;

    bool         CreateTextures(ATTRIBUTES* pATextureRoot);
    storm::FRect GetTextureRect(int textIdx, int picIdx) const;

    int m_nIconWidth;
    int m_nIconHeight;
    int m_nSpaceHorz;
    int m_nSpaceVert;

    int m_nShowPlaceQ;
    using _SHOW_PLACE = storm::FRect;
    _SHOW_PLACE* m_pShowPlaces;
    bool         CreateShowPlaces(ATTRIBUTES* pAPlacesRoot);
    void         RefreshShowPlaces(ATTRIBUTES* pAPlacesRoot);

    size_t m_nIShowQ;

    struct _PICTURE_DESCR {
        int32_t m_nPicNum;
        int32_t m_nPicTexIdx;
    }* m_pIconsList;

    bool InitIconsList(ATTRIBUTES* pAIconsRoot);
    void AddIconToList(ATTRIBUTES* pAIconDescr);
    void DelIconFromList(ATTRIBUTES* pAIconDescr);

    bool InitCommonBuffers();
};
