#include "xi_slide_picture.h"

#include <libs/renderer_next/impl_sdl/renderer_sdl.h>
#include <libs/renderer_next/ui/image_2d.h>

namespace
{

void set_texture_coordinate(storm::Image2D& image, FXYRECT tr, float angle)
{
    if (angle == 0) {
        image.set_uv(tr);
    } else {
        auto const x      = (tr.left + tr.right) * .5f;
        auto const y      = (tr.top + tr.bottom) * .5f;
        auto const width  = tr.right - tr.left;
        auto const height = tr.bottom - tr.top;
        auto const ca     = cosf(angle);
        auto const sa     = sinf(angle);
        auto const wca    = width / 2 * ca;
        auto const wsa    = width / 2 * sa;
        auto const hca    = height / 2 * ca;
        auto const hsa    = height / 2 * sa;
        image.set_uv_full({
            hlslpp::float2 {x + (-wca + hsa), y + (-wsa - hca)},
            hlslpp::float2 {x + (wca + hsa), y + (wsa - hca)},
            hlslpp::float2 {x + (wca - hsa), y + (wsa + hca)},
            hlslpp::float2 {x + (-wca - hsa), y + (-wsa + hca)},
        });
    }
}

}  // namespace

CXI_SLIDEPICTURE::CXI_SLIDEPICTURE() : minRotate(0), deltaRotate(0), curRotate(0), curAngle(0), nCurSlide(0)
{
    nLifeTime        = 0;
    m_nNodeType      = NODETYPE_SLIDEPICTURE;
    pSlideSpeedList  = nullptr;
    nSlideListSize   = 0;
    m_technique_name = "iVideo";
}

CXI_SLIDEPICTURE::~CXI_SLIDEPICTURE()
{
    ReleaseAll();
}

void CXI_SLIDEPICTURE::update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time)
{
    if (nCurSlide >= nSlideListSize) {
        return;
    }

    nLifeTime -= delta_time;
    if (nLifeTime < 0) {
        // changing speed
        nCurSlide++;
        if (nCurSlide >= nSlideListSize) {
            nCurSlide = 0;
        }
        nLifeTime = pSlideSpeedList[nCurSlide].time;
        curRotate = minRotate + ((static_cast<float>(rand()) / RAND_MAX) * deltaRotate);
    }

    auto const xadd = pSlideSpeedList[nCurSlide].xspeed * (delta_time / 1000.F);
    auto const yadd = pSlideSpeedList[nCurSlide].yspeed * (delta_time / 1000.F);

    curAngle += curRotate * (delta_time / 1000.F);

    m_texRect.left += xadd;
    m_texRect.right += xadd;
    m_texRect.top += yadd;
    m_texRect.bottom += yadd;

    while (m_texRect.left < -10) {
        m_texRect.left += 10;
        m_texRect.right += 10;
    }
    while (m_texRect.right > 10) {
        m_texRect.left -= 10;
        m_texRect.right -= 10;
    }
    while (m_texRect.top < -10) {
        m_texRect.top += 10;
        m_texRect.bottom += 10;
    }
    while (m_texRect.top > 10) {
        m_texRect.top -= 10;
        m_texRect.bottom -= 10;
    }

    set_texture_coordinate(*m_image, m_texRect, curAngle);
    m_image->update(copy_pass, delta_time);
}

void CXI_SLIDEPICTURE::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse) {
        m_image->draw(render_pass);
    }
}

bool CXI_SLIDEPICTURE::Init(
    INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
{
    if (!CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize)) {
        return false;
    }

    SetGlowCursor(false);
    return true;
}

void CXI_SLIDEPICTURE::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    int      i;
    char     param[255];
    FXYPOINT fPos;

    if (ReadIniString(ini1, name1, ini2, name2, "techniqueName", param, sizeof(param), "")) {
        m_technique_name = std::string(param);
    }

    m_texRect = GetIniFloatRect(ini1, name1, ini2, name2, "textureRect", FXYRECT(0.f, 0.f, 1.f, 1.f));

    m_color = GetIniARGB(ini1, name1, ini2, name2, "color", 0xFFFFFFFF);

    if (ReadIniString(ini1, name1, ini2, name2, "textureName", param, sizeof(param), "")) {
        SetNewPicture(param);
    }

    curAngle    = 0.f;
    curRotate   = 0.f;
    fPos        = GetIniFloatPoint(ini1, name1, ini2, name2, "rotate", FXYPOINT(0.f, 0.f));
    minRotate   = fPos.x;
    deltaRotate = fPos.y;

    nLifeTime       = 0;
    nCurSlide       = 0;
    nSlideListSize  = 0;
    pSlideSpeedList = nullptr;

    auto bUse1Ini = true;
    // Calculating the size of the speed table
    if (ini1->ReadString(name1, "speed", param, sizeof(param) - 1, "")) {
        do {
            nSlideListSize++;
        } while (ini1->ReadStringNext(name1, "speed", param, sizeof(param) - 1));
    } else {
        if (ini2->ReadString(name2, "speed", param, sizeof(param) - 1, "")) {
            bUse1Ini = false;
            do {
                nSlideListSize++;
            } while (ini2->ReadStringNext(name2, "speed", param, sizeof(param) - 1));
        }
    }

    if (nSlideListSize > 0) {
        pSlideSpeedList = new SLIDE_SPEED[nSlideListSize];
        if (pSlideSpeedList == nullptr) {
            throw std::runtime_error("allocate memory error");
        }
    }

    // fill in the speed table
    if (bUse1Ini) {
        ini1->ReadString(name1, "speed", param, sizeof(param) - 1, "");
        for (i = 0; i < nSlideListSize; i++) {
            pSlideSpeedList[i].time   = 0;
            pSlideSpeedList[i].xspeed = 0;
            pSlideSpeedList[i].yspeed = 0;
            GetDataStr(param, "lff", &pSlideSpeedList[i].time, &pSlideSpeedList[i].xspeed, &pSlideSpeedList[i].yspeed);
            ini1->ReadStringNext(name1, "speed", param, sizeof(param) - 1);
        }
    } else {
        ini2->ReadString(name2, "speed", param, sizeof(param) - 1, "");
        for (i = 0; i < nSlideListSize; i++) {
            pSlideSpeedList[i].time   = 0;
            pSlideSpeedList[i].xspeed = 0;
            pSlideSpeedList[i].yspeed = 0;
            GetDataStr(param, "lff", &pSlideSpeedList[i].time, &pSlideSpeedList[i].xspeed, &pSlideSpeedList[i].yspeed);
            ini2->ReadStringNext(name2, "speed", param, sizeof(param) - 1);
        }
    }
}

void CXI_SLIDEPICTURE::ReleaseAll()
{
    m_image.reset();
    STORM_DELETE(pSlideSpeedList);
    nSlideListSize = 0;
}

int CXI_SLIDEPICTURE::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_SLIDEPICTURE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return false;
}

void CXI_SLIDEPICTURE::ChangePosition(XYRECT& rNewPos)
{
    m_rect = rNewPos;
    if (m_image) {
        m_image->set_rect(m_rect);
    }
}

void CXI_SLIDEPICTURE::SaveParametersToIni()
{
    char pcWriteParam[2048];

    auto pIni = fio->open_ini_file(ptrOwner->m_sDialogFileName.c_str());
    if (!pIni) {
        core->Trace("Warning! Can`t open ini file name %s", ptrOwner->m_sDialogFileName.c_str());
        return;
    }

    // save position
    sprintf_s(pcWriteParam, sizeof(pcWriteParam), "%d,%d,%d,%d", m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
    pIni->WriteString(m_nodeName, "position", pcWriteParam);
}

void CXI_SLIDEPICTURE::SetNewPicture(char* sNewTexName)
{
    if (m_image) {
        m_image.reset();
    }

    m_image = std::make_unique<storm::Image2D>(sNewTexName, m_technique_name);
    m_image->set_uv(m_texRect);
    m_image->set_rect(m_rect);
    m_image->set_screen_rect(m_screen_rect);
    m_image->set_diffuse_color(storm::Color::from_hex(m_color));
}
