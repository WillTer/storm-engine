#pragma once

#include "../inode.h"

// text collection
class CXI_LINECOLLECTION: public CINODE
{
public:
    CXI_LINECOLLECTION();
    ~CXI_LINECOLLECTION() override;

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

protected:
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;
};
