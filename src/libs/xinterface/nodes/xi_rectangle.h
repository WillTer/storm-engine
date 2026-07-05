#pragma once

#include "../inode.h"

namespace storm
{
class Rectangle;
class Border;
}  // namespace storm

class CXI_RECTANGLE: public CINODE
{
public:
    CXI_RECTANGLE();
    ~CXI_RECTANGLE() override;

    void update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time) override;
    void Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time) override;
    bool
    Init(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
        override;
    void ReleaseAll() override;
    int  CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override {}

    void     ChangePosition(XYRECT& rNewPos) override;
    void     SaveParametersToIni() override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE& message) override;

    bool IsGlowChanged() override
    {
        return true;
    }

protected:
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;
    void UpdateColors();

    void create_border(uint32_t color);

    uint32_t m_dwTopColor;
    uint32_t m_dwBottomColor;
    uint32_t m_dwLeftColor;
    uint32_t m_dwRightColor;
    uint32_t m_dwBorderColor;

    std::unique_ptr<storm::Rectangle> m_back;
    std::unique_ptr<storm::Border>    m_border;
};
