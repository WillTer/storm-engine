#pragma once

#include "../inode.h"

namespace storm::renderer::ui
{
class Image2D;
}  // namespace storm::renderer::ui

// video
class CXI_VIDEO: public CINODE
{
public:
    CXI_VIDEO();
    ~CXI_VIDEO() override;

    void update(storm::GPUCopyPass const& copy_pass, bool is_selected, uint32_t delta_time) override;

    void Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time) override;
    bool
    Init(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
        override;
    void ReleaseAll() override;
    int  CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override {}

    void ChangePosition(XYRECT& rNewPos) override;
    void SaveParametersToIni() override;

protected:
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;

    std::unique_ptr<storm::renderer::ui::Image2D> m_video;

    uint32_t m_dwColor;
    FXYRECT  m_rectTex;
};
