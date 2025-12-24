#pragma once

#include <string>
#include <vector>

#include <libs/battle_interface/bi_defines.h>

class BIImageRender;
class BIImage;
class VAI_OBJBASE;

class ISpyglass: public Entity
{
    struct ImageParam {
        BIImage*     pImage;
        std::string  sTextureName;
        uint32_t     dwColor;
        storm::FRect rUV;
        storm::Rect  rPos;

        ImageParam()
        {
            pImage = nullptr;
        };

        ~ImageParam()
        {
            Release();
        }

        void Release();
        void LoadFromAttr(
            BIImageRender* pImgRender,
            ATTRIBUTES*    pA,
            char const*    pcDefName,
            int32_t        nDefLeftPos,
            int32_t        nDefTopPos,
            int32_t        nDefRightPos,
            int32_t        nDefBottomPos,
            int32_t        nPrior);
        void ChangeIcon(BIImageRender* pImgRender, char const* pcTextureName, storm::FRect& frUV);
    };

    struct TextParam {
        // VDX9RENDER* rs;
        int32_t      nFontID;
        storm::Point pos;
        float        fScale;
        uint32_t     dwColor;
        std::string  sText;
        int32_t      nAlign;

        TextParam(TextParam&&)      = delete;
        TextParam(TextParam const&) = delete;

        TextParam()
        {
            // rs      = nullptr;
            nFontID = -1;
        }

        ~TextParam()
        {
            Release();
        }

        void Release()
        {
            // if (rs && nFontID >= 0) rs->UnloadFont(nFontID);
            nFontID = -1;
        }

        void LoadFromAttr(/*VDX9RENDER*/ void* rs, ATTRIBUTES* pA, char const* pcDefText, int32_t nDefXPos, int32_t nDefYPos);
        void Print() const;
    };

    struct SpyGlassCameraParameters {
        float fSpyGlassPerspective;
        float fActivateTime;
        float fUpdateTime;

        float fOldPerspective;

        bool bIsActive;
        bool bIsGrow;

        float fCurUpdatingTime;
        float fCurActivateTime;
    };

public:
    ISpyglass();
    ~ISpyglass() override;
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
    // VDX9RENDER*    rs;
    BIImageRender* m_pImgRender;

    // telescope lens
    ImageParam m_Lens;

    // ship sign
    ImageParam m_ShipImage;  // ship back
    ImageParam m_ShipBack;   // ship back
    ImageParam m_ShipHP;     // ship HP
    ImageParam m_ShipSP;     // ship SP
    ImageParam m_Class;      // ship class

    // captain sign
    ImageParam m_CaptainBack;      // captain back image
    ImageParam m_CaptainFace;      // captain face image
    TextParam  m_TextCaptainName;  // captain name
    // captain skills
    ImageParam m_ImgCaptainFencing;
    TextParam  m_TextCaptainFencing;
    ImageParam m_ImgCaptainCannon;
    TextParam  m_TextCaptainCannon;
    ImageParam m_ImgCaptainAccuracy;
    TextParam  m_TextCaptainAccuracy;
    ImageParam m_ImgCaptainNavigation;
    TextParam  m_TextCaptainNavigation;
    ImageParam m_ImgCaptainBoarding;
    TextParam  m_TextCaptainBoarding;

    // nation flag
    ImageParam m_Nation;
    // cannon icon
    ImageParam m_Cannon;
    // sail icon (ship speed)
    ImageParam m_Sail;
    // cannon charge icon
    ImageParam m_Charge;

    // BOAL distance charge icon
    ImageParam m_SailTo;

    TextParam m_txtShipType;
    TextParam m_txtShipName;
    TextParam m_txtCannons;
    TextParam m_txtShipSpeed;
    TextParam m_txtShipCrew;
    TextParam m_txtSailTo;  // boal

    bool    m_bIsOn;
    bool    m_bIsPresentShipInfo;
    int32_t m_nInfoCharacterIndex;

    SpyGlassCameraParameters  m_Camera {};
    std::vector<storm::FRect> m_aNationUV;
    std::vector<storm::FRect> m_aChargeUV;
    std::vector<storm::FRect> m_aSailUV;

    VAI_OBJBASE* m_pFortObj;

    float m_fInfoKeepDelay;
    float m_fMaxInfoKeepDelay;

protected:
    void        Release();
    ATTRIBUTES* GetAttr(char const* pcAttrName) const;
    void        TurnOnTelescope(bool bTurnOn);
    void        SetShipInfo(int32_t nCharIndex);
    void        FindNewTargetShip();
    void        TurnOnSpyGlassCamera();
    void        ChangeTelescopeType(char const* pcTextureName, float fZoomScale, float fActivateTime, float fUpdateTime);
    void        UpdateCamera();
    void        ChangeTargetData(
               char const* pcShipName,
               char const* pcShipType,
               float       fRelativeHP,
               float       fRelativeSP,
               int32_t     nShipCrew,
               float       fShipSpeed,
               float       fSailTo,
               int32_t     nCurCannons,
               int32_t     nMaxCannons,
               int32_t     nCharge,
               int32_t     nNation,
               int32_t     nSailState,
               int32_t     nFace,
               int32_t     nFencing,
               int32_t     nCannon,
               int32_t     nAccuracy,
               int32_t     nNavigation,
               int32_t     nBoarding,
               char const* pcCaptainName,
               char const* pcFaceTexture,
               int32_t     nShipClass);
    void         FillUVArrayFromAttributes(std::vector<storm::FRect>& m_aUV, ATTRIBUTES* pA) const;
    VAI_OBJBASE* GetFort();
};
