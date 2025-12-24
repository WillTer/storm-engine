#pragma once

#include <string>
#include <vector>

#include <libs/battle_interface/bi_defines.h>

#define MAX_MAN_QUANTITY 10

class BIManCommandList;

class BIManSign
{
public:
    BIManSign(entid_t BIEntityID, /*VDX9RENDER*/ void* pRS);
    ~BIManSign();

    void Draw();
    void Init(ATTRIBUTES* pRoot, ATTRIBUTES* pA);

    size_t AddTexture(char const* pcTextureName, int32_t nCols, int32_t nRows) const;

    void Recollect();

    void SetUpdate()
    {
        m_bMakeUpdate = true;
    }

    bool IsActive() const;
    void SetActive(bool bActive);
    void MakeControl();
    void ExecuteCommand(int32_t command);

protected:
    void    Release();
    int32_t CalculateManQuantity();
    void    UpdateBuffers(int32_t nShipQ);
    void    FillIndexBuffer() const;
    void    FillVertexBuffer();
    int32_t
    WriteSquareToVBuff(BI_COLOR_VERTEX* pv, storm::FRect const& uv, uint32_t color, storm::FPoint const& center, storm::FPoint const& size);
    int32_t WriteSquareToVBuffWithProgress(
        BI_COLOR_VERTEX*     pv,
        storm::FRect const&  uv,
        uint32_t             color,
        storm::FPoint const& center,
        storm::FPoint const& size,
        float                fClampUp,
        float                fClampDown,
        float                fClampLeft,
        float                fClampRight);
    void UpdateCommandList() const;

    int32_t GetCurrentCommandTopLine() const;
    int32_t GetCurrentCommandCharacterIndex() const;
    int32_t GetCurrentCommandMode() const;

    float GetProgressManHP(int32_t nIdx) const;
    float GetProgressManEnergy(int32_t nIdx) const;
    float GetProgressGunChargeMax(int32_t nIdx);
    float GetProgressGunCharge(int32_t nIdx);
    float GetProgressGunReloadBar(int32_t nIdx);
    float GetGunProgressByIndex(int32_t nIdx);

    void CheckDataChange();

    bool     LongACompare(ATTRIBUTES* pA, char const* attrName, int32_t& nCompareVal);
    bool     FloatACompare(ATTRIBUTES* pA, char const* attrName, float& fCompareVal);
    bool     StringACompare(ATTRIBUTES* pA, char const* attrName, std::string& sCompareVal);
    bool     FRectACompare(ATTRIBUTES* pA, char const* attrName, storm::FRect& rCompareVal);
    bool     BoolACompare(ATTRIBUTES* pA, char const* attrName, bool& bCompareVal);
    uint32_t GetColorByFactor(uint32_t dwLowColor, uint32_t dwHighColor, float fFactor);

    // VDX9RENDER*       m_pRS;
    ATTRIBUTES*       m_pARoot;
    BIManCommandList* m_pCommandList;
    entid_t           m_idHostEntity;
    int32_t           m_nCommandMode;

    int32_t m_nVBufID;
    int32_t m_nIBufID;
    int32_t m_nSquareQ;

    int32_t m_nMaxSquareQ;

    int32_t       m_nBackTextureID;
    int32_t       m_nBackSquareQ;
    uint32_t      m_dwBackColor;
    storm::FRect  m_rBackUV;
    storm::FPoint m_pntBackOffset;
    storm::FPoint m_pntBackIconSize;

    bool          m_bIsAlarmOn;
    int32_t       m_nAlarmSquareQ;
    int32_t       m_nAlarmTextureID;
    uint32_t      m_dwAlarmHighColor;
    uint32_t      m_dwAlarmLowColor;
    storm::FRect  m_rAlarmUV;
    storm::FPoint m_pntAlarmOffset;
    storm::FPoint m_pntAlarmIconSize;
    bool          m_bAlarmUpDirection;
    float         m_fAlarmTime;
    float         m_fAlarmUpSpeed;
    float         m_fAlarmDownSpeed;

    int32_t       m_nManStateTextureID;
    int32_t       m_nManStateSquareQ;
    uint32_t      m_dwManStateColor;
    storm::FRect  m_rManHPUV;
    storm::FPoint m_pntManHPOffset;
    storm::FPoint m_pntManHPIconSize;
    storm::FRect  m_rManEnergyUV;
    storm::FPoint m_pntManEnergyOffset;
    storm::FPoint m_pntManEnergyIconSize;

    int32_t       m_nGunChargeTextureID;
    int32_t       m_nGunChargeSquareQ;
    uint32_t      m_dwGunChargeColor;
    uint32_t      m_dwGunChargeBackColor;
    storm::FRect  m_rGunChargeUV;
    storm::FPoint m_pntGunChargeOffset;
    storm::FPoint m_pntGunChargeIconSize;

    int32_t       m_nGunReloadTextureID;
    int32_t       m_nGunReloadSquareQ;
    uint32_t      m_dwGunReloadColor;
    uint32_t      m_dwGunReloadBackColor;
    storm::FRect  m_rGunReloadUV;
    storm::FPoint m_pntGunReloadOffset;
    storm::FPoint m_pntGunReloadIconSize;

    std::vector<float> m_aChargeProgress;

    storm::FRect  m_rManPicUV;
    storm::FPoint m_pntManPicOffset;
    storm::FPoint m_pntManPicIconSize;
    uint32_t      m_dwManFaceColor;

    struct ManDescr {
        storm::FPoint pntPos;  // center
        std::string   sTexture;
        int32_t       nTexture;
        storm::FRect  rUV;

        int32_t nSlotIndex;
        int32_t nCharacterIndex;
        float   fHealth;
        float   fEnergy;
        bool    bAlarm;
        int32_t nShootMax;
        int32_t nShootCurrent;
        float   fGunProgress;
    } m_Man[MAX_MAN_QUANTITY];

    int32_t m_nManQuantity;
    int32_t m_nCurrentManIndex;
    int32_t m_nCommandListVerticalOffset;

    bool m_bMakeUpdate;
    bool m_bMakeVertexFill;
    bool m_bActive;
};

inline bool BIManSign::LongACompare(ATTRIBUTES* pA, char const* attrName, int32_t& nCompareVal)
{
    auto const tmp = nCompareVal;
    nCompareVal    = pA->GetAttributeAsDword(attrName);
    return (nCompareVal != tmp);
}

inline bool BIManSign::FloatACompare(ATTRIBUTES* pA, char const* attrName, float& fCompareVal)
{
    auto const tmp = fCompareVal;
    fCompareVal    = pA->GetAttributeAsFloat(attrName);
    return (fCompareVal != tmp);
}

inline bool BIManSign::StringACompare(ATTRIBUTES* pA, char const* attrName, std::string& sCompareVal)
{
    char const* pVal = pA->GetAttribute(attrName);
    if (pVal == nullptr || sCompareVal == pVal) return false;
    sCompareVal = pVal;
    return true;
}

inline bool BIManSign::FRectACompare(ATTRIBUTES* pA, char const* attrName, storm::FRect& rCompareVal)
{
    char const* pVal = pA->GetAttribute(attrName);
    if (!pVal) return false;
    storm::FRect rTmp;
    rTmp.left   = rCompareVal.left;
    rTmp.top    = rCompareVal.top;
    rTmp.right  = rCompareVal.right;
    rTmp.bottom = rCompareVal.bottom;
    sscanf(pVal, "%f,%f,%f,%f", &rCompareVal.left, &rCompareVal.top, &rCompareVal.right, &rCompareVal.bottom);
    if (rCompareVal.left == rTmp.left && rCompareVal.top == rTmp.top && rCompareVal.right == rTmp.right
        && rCompareVal.bottom == rTmp.bottom)
        return false;
    return true;
}

inline bool BIManSign::BoolACompare(ATTRIBUTES* pA, char const* attrName, bool& bCompareVal)
{
    auto const tmp = bCompareVal;
    bCompareVal    = pA->GetAttributeAsDword(attrName, bCompareVal ? 1 : 0) != 0;
    return (bCompareVal != tmp);
}

inline uint32_t BIManSign::GetColorByFactor(uint32_t dwLowColor, uint32_t dwHighColor, float fFactor)
{
    int32_t asrc = (dwLowColor >> 24) & 0xFF;
    int32_t rsrc = (dwLowColor >> 16) & 0xFF;
    int32_t gsrc = (dwLowColor >> 8) & 0xFF;
    int32_t bsrc = dwLowColor & 0xFF;
    //
    int32_t const adst = (dwHighColor >> 24) & 0xFF;
    int32_t const rdst = (dwHighColor >> 16) & 0xFF;
    int32_t const gdst = (dwHighColor >> 8) & 0xFF;
    int32_t const bdst = dwHighColor & 0xFF;
    //
    asrc += static_cast<int32_t>((adst - asrc) * fFactor) & 0xFF;
    rsrc += static_cast<int32_t>((rdst - rsrc) * fFactor) & 0xFF;
    gsrc += static_cast<int32_t>((gdst - gsrc) * fFactor) & 0xFF;
    bsrc += static_cast<int32_t>((bdst - bsrc) * fFactor) & 0xFF;
    //
    return storm::Color {static_cast<uint8_t>(asrc), static_cast<uint8_t>(rsrc), static_cast<uint8_t>(gsrc), static_cast<uint8_t>(bsrc)}
        .to_hex();
}
