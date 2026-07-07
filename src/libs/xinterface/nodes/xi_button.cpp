#include "xi_button.h"

#include <libs/renderer_next/types.h>
#include <libs/renderer_next/ui/base.h>
#include <libs/renderer_next/ui/image_2d.h>
#include <libs/util/string_compare.hpp>

using namespace storm::renderer;

namespace
{
std::string const SHADOW_TECHNIQUE_NAME = "iShadow";
std::string const BUTTON_TECHNIQUE_NAME = "iButton";
}  // namespace

CXI_BUTTON::CXI_BUTTON()
{
    m_sGroupName = nullptr;
    m_idTex      = -1;

    fXShadow = 0.f;
    fYShadow = 0.f;

    nPressedDelay = 0;
    nMaxDelay     = 100;

    m_nFontNum = -1;

    m_bClickable = true;
    m_nNodeType  = NODETYPE_BUTTON;

    m_fBlindSpeed = -1.f;
}

CXI_BUTTON::~CXI_BUTTON()
{
    ReleaseAll();
}

void CXI_BUTTON::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse) {
        m_shadow->draw(render_pass);
        m_picture->draw(render_pass);

        // if (m_idString != -1L)
        //     if (nPressedDelay > 0) {
        //         m_rs->ExtPrint(
        //             m_nFontNum,
        //             m_dwFontColor,
        //             0,
        //             PR_ALIGN_CENTER,
        //             false,
        //             1.f,
        //             m_screenSize.x,
        //             m_screenSize.y,
        //             (m_rect.left + m_rect.right) / 2 + static_cast<int>(fXDeltaPress),
        //             m_rect.top + m_dwStrOffset + static_cast<int>(fYDeltaPress),
        //             "%s",
        //             pStringService->GetString(m_idString));
        //     } else {
        //         m_rs->ExtPrint(
        //             m_nFontNum,
        //             m_dwFontColor,
        //             0,
        //             PR_ALIGN_CENTER,
        //             false,
        //             1.f,
        //             m_screenSize.x,
        //             m_screenSize.y,
        //             (m_rect.left + m_rect.right) / 2,
        //             m_rect.top + m_dwStrOffset,
        //             "%s",
        //             pStringService->GetString(m_idString));
        //     }
    }
}

bool CXI_BUTTON::Init(
    INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
{
    return CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize);
}

void CXI_BUTTON::update(storm::GPUCopyPass const& copy_pass, bool is_selected, uint32_t delta_time)
{
    if (!m_picture) {
        return;
    }

    ChangePosition(m_rect);

    if (nPressedDelay > 0) {
        nPressedDelay--;
        m_picture->set_rect(m_rect_pressed);
        m_shadow->set_rect(m_shadow_rect_pressed);
    } else {
        m_picture->set_rect(m_rect);
        m_shadow->set_rect(m_shadow_rect);
    }

    auto face_color = m_dwFaceColor;
    if (is_selected && m_fBlindSpeed > 0.F) {
        face_color = ColorInterpolate(m_dwDarkColor, m_dwLightColor, m_fCurBlind);
        if (m_bUpBlind) {
            m_fCurBlind += m_fBlindSpeed * delta_time;
        } else {
            m_fCurBlind -= m_fBlindSpeed * delta_time;
        }

        if (m_fCurBlind < 0.F) {
            m_fCurBlind = 0.F;
            m_bUpBlind  = true;
        }
        if (m_fCurBlind > 1.F) {
            m_fCurBlind = 1.F;
            m_bUpBlind  = false;
        }
    }

    if (m_bClickable && m_bSelected) {
        m_picture->set_ubo_color(face_color);
    } else {
        m_picture->set_ubo_color(m_argbDisableColor);
    }

    m_picture->update(copy_pass, delta_time);
    m_shadow->update(copy_pass, delta_time);
}

void CXI_BUTTON::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    char    param[255];
    XYPOINT tmpLPnt;

    // get font number
    if (ReadIniString(ini1, name1, ini2, name2, "font", param, sizeof(param), "")) {
        // if ((m_nFontNum = m_rs->LoadFont(param)) == -1) core->Trace("can not load font:'%s'", param);
    }

    // get face color
    m_dwFaceColor = GetIniARGB(ini1, name1, ini2, name2, "faceColor", 0xFFFFFFFF);

    // selected color (light)
    m_dwLightColor = GetIniARGB(ini1, name1, ini2, name2, "lightColor", 0xFFFFFFFF);

    // (dark)
    m_dwDarkColor = GetIniARGB(ini1, name1, ini2, name2, "darkColor", 0xFFFFFFFF);

    // blinking speed
    m_fBlindSpeed = GetIniFloat(ini1, name1, ini2, name2, "blindTimeSec", -1.f);
    if (m_fBlindSpeed <= 0.F) {
        m_fBlindSpeed = 1.F;
    } else {
        m_fBlindSpeed = .001f / m_fBlindSpeed;
    }
    m_fCurBlind = 1.F;
    m_bUpBlind  = false;

    // get disable color
    m_argbDisableColor = GetIniARGB(ini1, name1, ini2, name2, "disableColor", storm::Color {255, 128, 128, 128}.to_hex());

    // get shadow color
    m_dwShadowColor = GetIniARGB(ini1, name1, ini2, name2, "shadowColor", storm::Color {255, 0, 0, 0}.to_hex());

    // get font color
    m_dwFontColor = GetIniARGB(ini1, name1, ini2, name2, "fontColor", 0xFFFFFFFF);

    // get group name and get texture for this
    if (ReadIniString(ini1, name1, ini2, name2, "group", param, sizeof(param), "")) {
        auto const len = strlen(param) + 1;
        m_sGroupName   = new char[len];
        if (m_sGroupName == nullptr) {
            throw std::runtime_error("allocate memory error");
        }
        memcpy(m_sGroupName, param, len);
        auto const texture = pPictureService->get_texture(m_sGroupName);

        // get button picture name
        if (ReadIniString(ini1, name1, ini2, name2, "picture", param, sizeof(param), "")) {
            m_picture = std::make_unique<ui::Image2D>(texture, pPictureService->get_texture_uv(m_sGroupName, param), BUTTON_TECHNIQUE_NAME);
            m_shadow  = std::make_unique<ui::Image2D>(texture, pPictureService->get_texture_uv(m_sGroupName, param), SHADOW_TECHNIQUE_NAME);
        } else {
            m_picture = std::make_unique<ui::Image2D>(texture, storm::FRect {}, BUTTON_TECHNIQUE_NAME);
            m_shadow  = std::make_unique<ui::Image2D>(texture, storm::FRect {}, SHADOW_TECHNIQUE_NAME);
        }
    } else if (ReadIniString(ini1, name1, ini2, name2, "videoTexture", param, sizeof(param), "")) {
        m_picture = std::make_unique<ui::Image2D>(pPictureService->get_video_texture(param), storm::FRect {}, BUTTON_TECHNIQUE_NAME);
        m_shadow  = std::make_unique<ui::Image2D>(pPictureService->get_video_texture(param), storm::FRect {}, SHADOW_TECHNIQUE_NAME);
    }

    assert(m_picture && m_shadow);
    m_picture->set_screen_rect(m_screen_rect);
    m_shadow->set_screen_rect(m_screen_rect);
    m_shadow->set_ubo_color(m_dwShadowColor);

    // get offset button image in case pressed button
    tmpLPnt      = GetIniLongPoint(ini1, name1, ini2, name2, "pressPictureOffset", XYPOINT(0, 0));
    fXDeltaPress = static_cast<float>(tmpLPnt.x);
    fYDeltaPress = static_cast<float>(tmpLPnt.y);

    // get offset button shadow in case pressed button
    tmpLPnt  = GetIniLongPoint(ini1, name1, ini2, name2, "shadowOffset", XYPOINT(0, 0));
    fXShadow = static_cast<float>(tmpLPnt.x);
    fYShadow = static_cast<float>(tmpLPnt.y);

    // get offset button shadow in case not pressed button
    tmpLPnt       = GetIniLongPoint(ini1, name1, ini2, name2, "pressShadowOffset", XYPOINT(0, 0));
    fXShadowPress = static_cast<float>(tmpLPnt.x);
    fYShadowPress = static_cast<float>(tmpLPnt.y);

    // get press delay
    nMaxDelay = GetIniLong(ini1, name1, ini2, name2, "pressDelay", 20);

    m_dwStrOffset = GetIniLong(ini1, name1, ini2, name2, "strOffset", 0);

    m_idString = -1;
    if (ReadIniString(ini1, name1, ini2, name2, "group", param, sizeof(param), "")) {
        m_idString = pStringService->GetStringNum(param);
    }
}

void CXI_BUTTON::ReleaseAll()
{
    PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName, m_idTex);
    delete[] m_sGroupName;
    m_sGroupName = nullptr;
}

int CXI_BUTTON::CommandExecute(int wActCode)
{
    if (m_bUse) {
        switch (wActCode) {
        case ACTION_ACTIVATE:
            nPressedDelay = nMaxDelay;
            break;
            // case ACTION_MOUSEDBLCLICK:
        case ACTION_MOUSECLICK:
            if (m_bClickable && m_bSelected) {
                nPressedDelay = nMaxDelay;
            }
            break;
        }
    }
    return -1;
}

bool CXI_BUTTON::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom && m_bClickable && m_bSelected
        && m_bUse;
}

void CXI_BUTTON::ChangePosition(XYRECT& rNewPos)
{
    m_rect = rNewPos;

    m_rect_pressed = {
        .left   = m_rect.left + fXDeltaPress,
        .top    = m_rect.top + fYDeltaPress,
        .right  = m_rect.right + fXDeltaPress,
        .bottom = m_rect.bottom + fYDeltaPress,
    };

    if (m_shadow) {
        m_shadow_rect = storm::FRect {
            .left   = m_rect.left + fXShadow,
            .top    = m_rect.top + fYShadow,
            .right  = m_rect.right + fXShadow,
            .bottom = m_rect.bottom + fYShadow,
        };

        m_shadow_rect_pressed = {
            .left   = m_rect.left + fXShadowPress,
            .top    = m_rect.top + fYShadowPress,
            .right  = m_rect.right + fXShadowPress,
            .bottom = m_rect.bottom + fYShadowPress,
        };
    }
}

void CXI_BUTTON::SaveParametersToIni()
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

void CXI_BUTTON::SetUsing(bool bUsing)
{
    m_bUse        = bUsing;
    nPressedDelay = 0;
}

uint32_t CXI_BUTTON::MessageProc(int32_t msgcode, MESSAGE& message)
{
    switch (msgcode) {
    case 0:  // change the position of the button
        m_rect.left   = message.Long();
        m_rect.top    = message.Long();
        m_rect.right  = message.Long();
        m_rect.bottom = message.Long();
        GetAbsoluteRect(m_rect, message.Long());
        break;

    case 1:  // change texture coordinates
        m_tRect.left   = message.Float();
        m_tRect.top    = message.Float();
        m_tRect.right  = message.Float();
        m_tRect.bottom = message.Float();
        break;

    case 2:  // change picture
    {
        std::string const& param = message.String();
        auto const         len   = param.size() + 1;
        if (len == 1) {
            break;
        }

        if (m_sGroupName == nullptr || !storm::iEquals(m_sGroupName, param)) {
            PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName, m_idTex);
            STORM_DELETE(m_sGroupName);

            m_sGroupName = new char[len];
            if (m_sGroupName == nullptr) {
                throw std::runtime_error("allocate memory error");
            }
            memcpy(m_sGroupName, param.c_str(), len);
        }

        std::string const& param2 = message.String();

        m_picture = std::make_unique<ui::Image2D>(
            pPictureService->get_texture(m_sGroupName), pPictureService->get_texture_uv(m_sGroupName, param2), BUTTON_TECHNIQUE_NAME);
        m_shadow = std::make_unique<ui::Image2D>(
            pPictureService->get_texture(m_sGroupName), pPictureService->get_texture_uv(m_sGroupName, param2), SHADOW_TECHNIQUE_NAME);
        m_picture->set_screen_rect(m_screen_rect);
        m_shadow->set_screen_rect(m_screen_rect);
        m_shadow->set_ubo_color(m_dwShadowColor);
    } break;
    }

    return 0;
}
