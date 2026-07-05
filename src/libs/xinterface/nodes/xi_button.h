#pragma once

#include "../inode.h"

namespace storm
{
class Image2D;
}  // namespace storm

// picture
class CXI_BUTTON: public CINODE
{
public:
    CXI_BUTTON(CXI_BUTTON&&)      = delete;
    CXI_BUTTON(const CXI_BUTTON&) = delete;
    CXI_BUTTON();
    ~CXI_BUTTON() override;

    void Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time) override;
    bool
    Init(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
        override;
    void update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time) override;
    void ReleaseAll() override;
    int  CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override {}

    void ChangePosition(XYRECT& rNewPos) override;
    void SaveParametersToIni() override;

    void NotUsingTime(uint32_t Delta_Time)
    {
        nPressedDelay = 0;
    }

    void     SetUsing(bool bUsing) override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE& message) override;

protected:
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;

protected:
    std::unique_ptr<storm::Image2D> m_picture;
    std::unique_ptr<storm::Image2D> m_shadow;

    storm::FRect m_rect_pressed;
    storm::FRect m_shadow_rect;
    storm::FRect m_shadow_rect_pressed;

    char*   m_sGroupName;
    int32_t m_idTex;  // texture identity

    FXYRECT m_tRect;

    uint32_t m_argbDisableColor;
    uint32_t m_dwShadowColor;
    uint32_t m_dwFaceColor;
    uint32_t m_dwFontColor;
    uint32_t m_dwLightColor;
    uint32_t m_dwDarkColor;
    float    m_fBlindSpeed;
    float    m_fCurBlind;
    bool     m_bUpBlind;

    float fXShadow;
    float fYShadow;
    float fXShadowPress;
    float fYShadowPress;

    float fXDeltaPress;
    float fYDeltaPress;

    int nPressedDelay;
    int nMaxDelay;

    int     m_nFontNum;
    int32_t m_idString;
    int     m_dwStrOffset;
};
