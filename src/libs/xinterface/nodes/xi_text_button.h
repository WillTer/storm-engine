#pragma once

#include "../inode.h"

namespace storm
{
class Picture;
class Button;
}  // namespace storm

// picture
class CXI_TEXTBUTTON: public CINODE
{
public:
    CXI_TEXTBUTTON(CXI_TEXTBUTTON&&)      = delete;
    CXI_TEXTBUTTON(const CXI_TEXTBUTTON&) = delete;
    CXI_TEXTBUTTON();
    ~CXI_TEXTBUTTON() override;

    void Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time) override;
    bool
    Init(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
        override;
    void update(storm::GPUCopyPass const& copy_pass) override;
    void ReleaseAll() override;
    int  CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override {}

    void     ChangePosition(XYRECT& rNewPos) override;
    void     SaveParametersToIni() override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE& message) override;

    void NotUsingTime(uint32_t Delta_Time)
    {
        m_nPressedDelay = 0;
    }

    void SetUsing(bool bUsing) override;
    void MakeLClickPreaction() override;

protected:
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;
    void FillPositionIntoVertices();

protected:
    char*   m_sGroupName;
    int32_t m_idTex;        // texture identity
    int32_t m_idShadowTex;  // shadow texture

    struct UV {
        storm::FRect left_uv;
        storm::FRect middle_uv;
        storm::FRect right_uv;
    };

    std::shared_ptr<storm::GPUTexture> m_texture;

    UV m_uv;
    UV m_uv_selected;

    std::unique_ptr<storm::Button> m_button;
    std::unique_ptr<storm::Button> m_button_selected;

    std::string m_left_picture_name;
    std::string m_right_picture_name;

    bool m_bCurrentSelected;

    XI_ONETEX_VERTEX m_v[8];

    uint32_t m_dwShadowColor;        // shadow color
    uint32_t m_dwFaceColor;          // unpressed key color
    uint32_t m_dwPressedFaceColor;   // active key color
    uint32_t m_dwFontColor;          // font color
    uint32_t m_dwUnselFontColor;     // font color for unavailable key
    uint32_t m_dwPressedFontColor;   // font color for pressed button
    uint32_t m_dwSelectedFontColor;  // font color for selected button
    float    m_fFontScale;

    float m_fXShadow;       // X offset for normal key shadow
    float m_fYShadow;       // Y offset for normal key shadow
    float m_fXShadowPress;  // X offset for the shadow of the pressed key
    float m_fYShadowPress;  // Y offset for the shadow of the pressed key
    float m_fXDeltaPress;   // X offset on button click
    float m_fYDeltaPress;   // Y offset when button is pressed

    int m_nPressedDelay;
    int m_nMaxDelay;

    // string parameters
    int     m_nFontNum;
    int32_t m_idString;
    int     m_dwStrOffset;
    char*   m_sString;

    // picture index & vertex buffers
    int32_t m_idVBuf;  // identificator of the vertex buffer
    int32_t m_idIBuf;  // identificator of the index buffer
    int32_t m_nVert;   // vertex quantity
    int32_t m_nIndx;   // index quantity

    bool     m_bVideoToBack;
    uint32_t m_dwBackColor;

    float m_fShadowScale;
};
