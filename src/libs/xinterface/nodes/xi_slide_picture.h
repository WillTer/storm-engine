#pragma once

#include "../inode.h"

namespace storm
{
class Image2D;
class GPUSampler;
}  // namespace storm

// video
class CXI_SLIDEPICTURE: public CINODE
{
public:
    CXI_SLIDEPICTURE();
    ~CXI_SLIDEPICTURE() override;

    void update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time) override;
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

    void SetNewPicture(char* sNewTexName);

protected:
    void    LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;
    FXYRECT m_texRect;

    float minRotate;
    float deltaRotate;
    float curRotate;
    float curAngle;

    int32_t nLifeTime;
    int32_t nCurSlide;

    struct SLIDE_SPEED {
        uint32_t time;
        float    xspeed;
        float    yspeed;
    }* pSlideSpeedList;

    int32_t nSlideListSize;

    char* strTechniqueName;

    uint32_t                           m_color;
    std::unique_ptr<storm::Image2D>    m_image;
    std::shared_ptr<storm::GPUSampler> m_sampler;
};
