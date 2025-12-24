#pragma once

#include <string>
#include <vector>

#include "bi_defines.h"

class BIImageRender;

// nCommandType
#define BISCT_Command 1
#define BISCT_Ability 2
#define BISCT_Ship 3
#define BISCT_Fort 4
#define BISCT_Land 5
#define BISCT_Charge 6
#define BISCT_UserIcon 7
#define BISCT_Cancel 8

class BICommandList
{
public:
    BICommandList(BICommandList&&)      = delete;
    BICommandList(BICommandList const&) = delete;
    BICommandList(entid_t eid, ATTRIBUTES* pA, /*VDX9RENDER*/ void* rs);
    virtual ~BICommandList();

    void         Draw();
    void         Update(int32_t nTopLine, int32_t nCharacterIndex, int32_t nCommandMode);
    virtual void FillIcons() = 0;

    size_t AddTexture(char const* pcTextureName, uint32_t nCols, uint32_t nRows);

    // commands
    int32_t ExecuteConfirm();
    int32_t ExecuteLeft();
    int32_t ExecuteRight();
    int32_t ExecuteCancel();

    void SetActive(bool bActive);

    bool GetActive() const
    {
        return m_bActive;
    }

    void SetUpDown(bool bUp, bool bDown);

    virtual void Init();

    int32_t AddToIconList(
        int32_t     nTextureNum,
        int32_t     nNormPictureNum,
        int32_t     nSelPictureNum,
        int32_t     nCooldownPictureNum,
        int32_t     nCharacterIndex,
        char const* pcCommandName,
        int32_t     nTargetIndex,
        char const* pcLocName,
        char const* pcNoteName);
    void AddAdditiveToIconList(int32_t nTextureNum, int32_t nPictureNum, float fDist, float fWidth, float fHeight);

protected:
    entid_t     m_idHostObj;
    ATTRIBUTES* m_pARoot;
    // FIXME: Renderer Next
    // VDX9RENDER* m_pRS;

    BIImageRender* m_pImgRender;

    struct TextureDescr {
        std::string sFileName;
        uint32_t    nCols;
        uint32_t    nRows;
    };

    std::vector<TextureDescr> m_aTexture;

    struct UsedCommand {
        int32_t     nCharIndex;
        std::string sCommandName;
        int32_t     nTargetIndex;
        std::string sLocName;
        std::string sNote;

        int32_t nTextureIndex;
        int32_t nSelPictureIndex;
        int32_t nNormPictureIndex;
        int32_t nCooldownPictureIndex;

        float fCooldownFactor;

        struct AdditiveIcon {
            int32_t       nTex;
            int32_t       nPic;
            float         fDelta;
            storm::FPoint fpSize;
        };

        std::vector<AdditiveIcon> aAddPicList;
    };

    bool m_bActive;

    std::vector<UsedCommand> m_aUsedCommand;
    int32_t                  m_nStartUsedCommandIndex;
    int32_t                  m_nSelectedCommandIndex;
    int32_t                  m_nIconShowMaxQuantity;

    storm::Point m_pntActiveIconOffset;
    storm::Point m_pntActiveIconSize;
    std::string  m_sActiveIconTexture;
    storm::FRect m_frActiveIconUV1;
    storm::FRect m_frActiveIconUV2;
    std::string  m_sActiveIconNote;

    bool         m_bUpArrow;
    bool         m_bDownArrow;
    std::string  m_sUpDownArrowTexture;
    storm::FRect m_frUpArrowUV;
    storm::FRect m_frDownArrowUV;
    storm::Point m_pntUpDownArrowSize;
    storm::Point m_pntUpArrowOffset;
    storm::Point m_pntDownArrowOffset;

    bool         m_bLeftArrow;
    bool         m_bRightArrow;
    std::string  m_sLeftRightArrowTexture;
    storm::FRect m_frLeftArrowUV;
    storm::FRect m_frRightArrowUV;
    storm::Point m_pntLeftRightArrowSize;
    storm::Point m_pntLeftArrowOffset;
    storm::Point m_pntRightArrowOffset;

    std::string m_sCurrentCommandName;
    int32_t     m_nCurrentCommandCharacterIndex;
    int32_t     m_nCurrentCommandMode;

    storm::Point m_LeftTopPoint;
    storm::Point m_IconSize;
    int32_t      m_nIconSpace;

    int32_t      m_NoteFontID;
    uint32_t     m_NoteFontColor;
    float        m_NoteFontScale;
    storm::Point m_NotePos;
    storm::Point m_NoteOffset;
    std::string  m_NoteText;

    struct CoolDownUpdateData {
        int32_t nIconNum;
        float   fTime;
        float   fUpdateTime;
    };

    std::vector<CoolDownUpdateData> m_aCooldownUpdate;

    void Release();

    int32_t       IconAdd(int32_t nPictureNum, int32_t nTextureNum, storm::Rect& rpos);
    int32_t       ClockIconAdd(int32_t nForePictureNum, int32_t nBackPictureNum, int32_t nTextureNum, storm::Rect& rpos, float fFactor);
    void          AdditiveIconAdd(float fX, float fY, std::vector<UsedCommand::AdditiveIcon>& aList);
    storm::FRect& GetPictureUV(int32_t nTextureNum, int32_t nPictureNum, storm::FRect& uv);
    storm::Rect&  GetCurrentPos(int32_t num, storm::Rect& rpos) const;
    storm::Rect&  GetAddingPos(int32_t num, storm::Rect& rpos);

    void UpdateShowIcon();
    void SetNote(char const* pcNote, int32_t nX, int32_t nY);

    ATTRIBUTES* GetCurrentCommandAttribute() const;
};
