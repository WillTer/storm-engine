#pragma once

#include <string>
#include <vector>

#include <libs/core/entity.h>
#include <libs/math/matrix.h>

#include "link_describe.hpp"

constexpr auto MAX_LINES         = 5;
constexpr auto SCROLL_LINE_TIME  = 100;
constexpr auto TILED_LINE_HEIGHT = 26;
constexpr auto SBL               = 6;
#define TICK_SOUND "interface/ok.wav"

#define XI_TEX_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2)
class INIFILE;

struct XI_TEX_VERTEX {
    CVECTOR  pos;
    float    rhw;
    uint32_t color;
    float    u, v;
};

constexpr auto BUTTON_STATE_UPENABLE   = 1;
constexpr auto BUTTON_STATE_DOWNENABLE = 2;
constexpr auto BUTTON_STATE_UPLIGHT    = 4;
constexpr auto BUTTON_STATE_DOWNLIGHT  = 8;

class VSoundService;

class Dialog final: public Entity
{
    // static VDX9RENDER* RenderService;

public:
    Dialog(Dialog&&)      = delete;
    Dialog(Dialog const&) = delete;
    Dialog();
    ~Dialog();

    bool     Init() override;
    void     InitLinks(INIFILE* pIni);
    void     Realize(uint32_t Delta_Time);
    uint32_t AttributeChanged(ATTRIBUTES* pA);
    uint64_t ProcessMessage(MESSAGE& message);

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
            // case Stage::execute:
            //    Execute(delta); break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
                LostRender(delta); break;
            case Stage::restore_render:
                RestoreRender(delta); break;*/
        }
    }

private:
    void EmergencyExit();

    // Nikita data
    std::string m_sTalkPersName;

    bool m_bDlgChanged;
    void UpdateDlgTexts();
    void UpdateDlgViewport();

    struct DlgTextDescribe {
    private:
        storm::Point             offset;
        int32_t                  nWindowWidth;
        int32_t                  nFontID;
        uint32_t                 dwColor;
        float                    fScale;
        int32_t                  nLineInterval;
        std::vector<std::string> asText;
        int32_t                  nShowQuantity;

        std::vector<int32_t> pageBreaks_;

    public:
        int32_t currentLine_;

        ~DlgTextDescribe()
        {
            // if (RenderService && nFontID >= 0) RenderService->UnloadFont(nFontID);
        }

        void    ChangeText(std::string_view text);
        void    Init(INIFILE* pIni);
        int32_t GetShowHeight();
        void    Show(int32_t nY);
        bool    IsLastPage();
        void    PrevPage();
        void    NextPage();
    };

    DlgTextDescribe m_DlgText;

    storm::dialog::DlgLinkDescribe linkDescribe_;

    struct BackParameters {
        int32_t m_idBackTex;

        storm::FRect m_frLeftTopUV;
        storm::FRect m_frRightTopUV;
        storm::FRect m_frLeftBottomUV;
        storm::FRect m_frRightBottomUV;
        storm::FRect m_frLeftUV;
        storm::FRect m_frRightUV;
        storm::FRect m_frTopUV;
        storm::FRect m_frBottomUV;
        storm::FRect m_frCenterUV;
        storm::FRect m_frDividerUV;

        storm::FRect m_frBorderExt;
        storm::FRect m_frBorderInt;
        storm::FRect frBorderRect;

        storm::FRect  frCharacterNameRectLeftUV;
        storm::FRect  frCharacterNameRectRightUV;
        storm::FRect  frCharacterNameRectCenterUV;
        storm::FPoint fpCharacterNameOffset;
        float         fCharacterNameRectHeight;
        float         fCharacterNameRectLeftWidth;
        float         fCharacterNameRectCenterWidth;
        float         fCharacterNameRectRightWidth;

        bool    bShowDivider;
        float   nDividerHeight;
        float   nDividerOffsetX;
        int32_t nDividerOffsetY;
    };

    BackParameters m_BackParams;
    int32_t        m_idVBufBack;
    int32_t        m_idIBufBack;
    int32_t        m_nVQntBack;
    int32_t        m_nIQntBack;

    struct ButtonParameters {
        int32_t m_idTexture;

        storm::FRect frUpNormalButtonUV;
        storm::FRect frDownNormalButtonUV;
        storm::FRect frUpLightButtonUV;
        storm::FRect frDownLightButtonUV;

        storm::FPoint fpButtonSize;

        float fRightOffset;
        float fTopOffset;
        float fBottomOffset;
    };

    ButtonParameters m_ButtonParams;
    int32_t          m_idVBufButton;
    int32_t          m_idIBufButton;
    int32_t          m_nVQntButton;
    int32_t          m_nIQntButton;
    uint32_t         m_dwButtonState;

    int32_t       m_nCharNameTextFont;
    uint32_t      m_dwCharNameTextColor;
    float         m_fCharNameTextScale;
    storm::FPoint m_fpCharNameTextOffset;

    int32_t             m_nScrBaseWidth;
    int32_t             m_nScrBaseHeight;
    static storm::FRect m_frScreenData;

    static float GetScrX(float fX)
    {
        return fX * m_frScreenData.right + m_frScreenData.left;
    }

    static float GetScrY(float fY)
    {
        return fY * m_frScreenData.bottom + m_frScreenData.top;
    }

    static float GetScrWidth(float fX)
    {
        return fX * m_frScreenData.right;
    }

    static float GetScrHeight(float fY)
    {
        return fY * m_frScreenData.bottom;
    }

    void CreateBack();
    void FillBack();
    void FillDivider();
    void DrawBack();

    void CreateButtons();
    void FillButtons();
    void DrawButtons();

    void LoadFromIni();

    static void GetRectFromIni(INIFILE* ini, char const* pcSection, char const* pcKey, storm::FRect& frect);
    static void GetPointFromIni(INIFILE* ini, char const* pcSection, char const* pcKey, storm::FPoint& fpoint);

    VSoundService* snd;
    entid_t        charId, persId;
    entid_t        charMdl, persMdl;
    // D3DVIEWPORT9   textViewport;

    int32_t curSnd;
    char    soundName[256];
    char    charDefSnd[256];

    bool forceEmergencyClose;
    char selectedLinkName[1024];

    int unfadeTime;

    int32_t play;
    bool    start;

    bool bEditMode;
};
