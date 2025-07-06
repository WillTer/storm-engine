#include "xi_text_button.h"

#include <libs/renderer_next/types.h>
#include <libs/renderer_next/ui/button.h>
#include <libs/renderer_next/ui/colored_rect.h>
#include <libs/renderer_next/ui/font.h>
#include <libs/renderer_next/ui/image_2d.h>

CXI_TEXTBUTTON::CXI_TEXTBUTTON()
{
    m_sGroupName  = nullptr;
    m_idTex       = -1;
    m_idShadowTex = -1;

    m_fXShadow = 0.f;
    m_fYShadow = 0.f;

    m_nPressedDelay = 0;
    m_nMaxDelay     = 100;

    m_nFontNum   = -1;
    m_fFontScale = 1.f;

    m_bClickable   = true;
    m_bSelected    = true;
    m_bMouseSelect = true;

    m_nNodeType    = NODETYPE_TEXTBUTTON;
    m_sString      = nullptr;
    m_bVideoToBack = true;

    m_dwBackColor = storm::Color {128, 0, 0, 0}.to_hex();

    m_selection = nullptr;
    m_back      = nullptr;
    m_font      = nullptr;
    m_text      = nullptr;
}

CXI_TEXTBUTTON::~CXI_TEXTBUTTON()
{
    ReleaseAll();
}

void CXI_TEXTBUTTON::pre_draw(storm::GPUCommandBuffer const& cmd_buffer, uint32_t delta_time)
{
    if (m_font && !m_text && (m_idString != -1 || m_sString != nullptr)) {
        m_text = std::make_unique<storm::Image2D>(m_font->print(
            cmd_buffer,
            storm::Color::from_hex(0xFFFFFFFF),
            storm::Color::from_hex(0),
            storm::Font::Alignment::Center,
            true,  // draw_shadow
            m_fFontScale,
            m_rect.right - m_rect.left,
            m_rect.bottom - m_rect.top,
            m_idString != -1 ? pStringService->GetString(m_idString) : m_sString));
        m_text->set_screen_rect(m_screen_rect);
    }
}

void CXI_TEXTBUTTON::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse) {
        // show shadow
        if (m_shadow) { m_shadow->draw(render_pass); }

        if (m_bVideoToBack) {
            // show midle video fragment
            if (bSelected && m_selection) {
                m_selection->draw(render_pass);
            } else if (m_dwBackColor != 0) {
                m_back->draw(render_pass);
            }
        }

        // show button
        if (bSelected) {
            m_button_selected->draw(render_pass);
        } else {
            m_button->draw(render_pass);
        }

        if (!m_bVideoToBack) {
            if (bSelected && m_selection) { m_selection->draw(render_pass); }
        }

        if (m_text) { m_text->draw(render_pass); }
    }
}

bool CXI_TEXTBUTTON::Init(
    INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
{
    return CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize);
}

void CXI_TEXTBUTTON::update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time)
{
    if (!m_bMakeActionInDeclick && m_nPressedDelay > 0) { m_nPressedDelay--; }

    ChangePosition(m_rect);
    if (m_nPressedDelay > 0) {
        m_back->set_rect(m_rect_pressed);
        m_button->set_rect(m_rect_pressed);
        m_button_selected->set_rect(m_rect_pressed);
        if (m_shadow) { m_shadow->set_rect(m_shadow_rect_pressed); }
        if (m_selection) { m_selection->set_rect(m_rect_pressed); }
        if (m_text) { m_text->set_rect(m_rect_pressed); }
    } else {
        m_back->set_rect(m_rect);
        m_button->set_rect(m_rect);
        m_button_selected->set_rect(m_rect);
        if (m_shadow) { m_shadow->set_rect(m_shadow_rect); }
        if (m_selection) { m_selection->set_rect(m_rect); }
        if (m_text) { m_text->set_rect(m_rect); }
    }

    m_back->update(copy_pass, delta_time);
    m_button->update(copy_pass, delta_time);
    m_button_selected->update(copy_pass, delta_time);
    if (m_shadow) { m_shadow->update(copy_pass, delta_time); }
    if (m_selection) { m_selection->update(copy_pass, delta_time); }
    if (m_text) { m_text->update(copy_pass, delta_time); }

    if (m_nPressedDelay > 0) {
        m_button->set_diffuse_color(storm::Color::from_hex(m_dwPressedFaceColor));
        m_button_selected->set_diffuse_color(storm::Color::from_hex(m_dwPressedFaceColor));
        if (m_text) { m_text->set_diffuse_color(storm::Color::from_hex(m_dwPressedFontColor)); }
    } else {
        m_button->set_diffuse_color(storm::Color::from_hex(m_dwFaceColor));
        m_button_selected->set_diffuse_color(storm::Color::from_hex(m_dwFaceColor));

        if (m_text) {
            if (m_bSelected) {
                m_text->set_diffuse_color(storm::Color::from_hex(m_dwFontColor));
            } else {
                m_text->set_diffuse_color(storm::Color::from_hex(m_dwUnselFontColor));
            }
        }
    }
}

void CXI_TEXTBUTTON::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    char param[255];
    // video to back
    m_bVideoToBack = GetIniBool(ini1, name1, ini2, name2, "videoToBack", true);

    // get back color for button
    m_dwBackColor = GetIniARGB(ini1, name1, ini2, name2, "backColor", storm::Color {255, 255, 255, 255}.to_hex());

    // get face color for not pressed button
    m_dwFaceColor = GetIniARGB(ini1, name1, ini2, name2, "faceColor", storm::Color {255, 255, 255, 255}.to_hex());

    // get face color for pressed button
    m_dwPressedFaceColor = GetIniARGB(ini1, name1, ini2, name2, "faceColorPressed", storm::Color {255, 255, 255, 255}.to_hex());

    // get shadow color
    m_dwShadowColor = GetIniARGB(ini1, name1, ini2, name2, "shadowColor", storm::Color {255, 0, 0, 0}.to_hex());

    // get font color
    m_dwFontColor = GetIniARGB(ini1, name1, ini2, name2, "fontColor", storm::Color {255, 255, 255, 255}.to_hex());

    // get font color for pressed button
    m_dwPressedFontColor = GetIniARGB(ini1, name1, ini2, name2, "fontColorPressed", m_dwFontColor);

    // get font color for selected button
    m_dwSelectedFontColor = GetIniARGB(ini1, name1, ini2, name2, "fontColorSelected", m_dwFontColor);

    m_fFontScale = GetIniFloat(ini1, name1, ini2, name2, "fontScale", 1.f);

    // get deselected font color
    m_dwUnselFontColor = GetIniARGB(ini1, name1, ini2, name2, "unselectableFontColor", storm::Color {255, 128, 128, 128}.to_hex());

    // get group name and get texture for this
    m_sGroupName = nullptr;
    m_idTex      = -1;

    std::shared_ptr<storm::GPUTexture> button_texture = nullptr;
    if (ReadIniString(ini1, name1, ini2, name2, "group", param, sizeof(param), "")) {
        auto const len = strlen(param) + 1;
        m_sGroupName   = new char[len];
        if (m_sGroupName == nullptr) throw std::runtime_error("allocate memory error");
        memcpy(m_sGroupName, param, len);
        button_texture = pPictureService->get_texture(m_sGroupName);
    }

    m_back = std::make_unique<storm::ColoredRect>(storm::Color::from_hex(m_dwBackColor));
    m_back->set_screen_rect(m_screen_rect);

    m_idShadowTex = -1;
    if (ReadIniString(ini1, name1, ini2, name2, "ShadowTexture", param, sizeof(param), "")) {
        auto const shadow_uv = GetIniFloatRect(ini1, name1, ini2, name2, "ShadowUV", FXYRECT(0.F, 0.F, 1.F, 1.F));

        m_shadow = std::make_unique<storm::Image2D>(param, shadow_uv);
        m_shadow->set_screen_rect(m_screen_rect);
        m_shadow->set_diffuse_color(storm::Color::from_hex(m_dwShadowColor));
    }

    // get offset button image in case pressed button
    FXYPOINT offset = GetIniFloatPoint(ini1, name1, ini2, name2, "pressPictureOffset", FXYPOINT(0.f, 0.f));
    m_fXDeltaPress  = offset.x;
    m_fYDeltaPress  = offset.y;

    // get offset button shadow in case pressed button
    offset     = GetIniFloatPoint(ini1, name1, ini2, name2, "shadowOffset", FXYPOINT(0.f, 0.f));
    m_fXShadow = offset.x;
    m_fYShadow = offset.y;

    // get offset button shadow in case not pressed button
    offset          = GetIniFloatPoint(ini1, name1, ini2, name2, "pressShadowOffset", FXYPOINT(0.f, 0.f));
    m_fXShadowPress = offset.x;
    m_fYShadowPress = offset.y;

    // get press delay
    m_nMaxDelay = GetIniLong(ini1, name1, ini2, name2, "pressDelay", 20);

    // get string parameters
    if (ReadIniString(ini1, name1, ini2, name2, "font", param, sizeof(param), "")) { m_font = std::make_unique<storm::Font>(param); }
    //     if ((m_nFontNum = m_rs->LoadFont(param)) == -1) core->Trace("can not load font:'%s'", param);
    m_dwStrOffset = GetIniLong(ini1, name1, ini2, name2, "strOffset", 0);

    m_idString = -1;
    if (ReadIniString(ini1, name1, ini2, name2, "string", param, sizeof(param), "")) { m_idString = pStringService->GetStringNum(param); }

    m_fShadowScale = GetIniFloat(ini1, name1, ini2, name2, "shadowScale", 1.F);

    // get video fragment parameters
    if (ReadIniString(ini1, name1, ini2, name2, "midVideo", param, sizeof(param), "")) {
        m_selection = std::make_unique<storm::Image2D>(pPictureService->get_video_texture(param));
        m_selection->set_screen_rect(m_screen_rect);
        m_selection->set_diffuse_color(storm::Color::from_hex(m_dwFaceColor) * 2);
    }

    // fill left side of button
    auto const left_uv_selected = ReadIniString(ini1, name1, ini2, name2, "selectButtonLeft", param, sizeof(param), "")
        ? pPictureService->get_texture_uv(m_sGroupName, param)
        : storm::FRect {};

    auto const left_uv = ReadIniString(ini1, name1, ini2, name2, "buttonLeft", param, sizeof(param), "")
        ? pPictureService->get_texture_uv(m_sGroupName, param)
        : storm::FRect {};

    // // fill right side of button
    auto const right_uv_selected = ReadIniString(ini1, name1, ini2, name2, "selectButtonRight", param, sizeof(param), "")
        ? pPictureService->get_texture_uv(m_sGroupName, param)
        : storm::FRect {};

    auto right_uv = ReadIniString(ini1, name1, ini2, name2, "buttonRight", param, sizeof(param), "")
        ? pPictureService->get_texture_uv(m_sGroupName, param)
        : storm::FRect {};

    // fill middle of button
    auto const middle_uv = ReadIniString(ini1, name1, ini2, name2, "buttonMiddle", param, sizeof(param), "")
        ? pPictureService->get_texture_uv(m_sGroupName, param)
        : storm::FRect {};

    auto const middle_uv_selected = ReadIniString(ini1, name1, ini2, name2, "selectButtonMiddle", param, sizeof(param), "")
        ? pPictureService->get_texture_uv(m_sGroupName, param)
        : storm::FRect {};

    if (right_uv.is_empty()) {
        right_uv = storm::FRect {
            .left   = left_uv.right,  // Mirror x
            .top    = left_uv.top,
            .right  = left_uv.left,
            .bottom = left_uv.bottom,
        };
    }

    m_button = std::make_unique<storm::Button>(button_texture, left_uv, middle_uv, right_uv, m_rect);
    m_button->set_screen_rect(m_screen_rect);
    m_button->set_diffuse_color(storm::Color::from_hex(m_dwFaceColor));

    m_button_selected = std::make_unique<storm::Button>(
        button_texture,
        !left_uv_selected.is_empty() ? left_uv_selected : left_uv,
        !middle_uv_selected.is_empty() ? middle_uv_selected : middle_uv,
        !right_uv_selected.is_empty() ? right_uv_selected : right_uv,
        m_rect);

    m_button_selected->set_screen_rect(m_screen_rect);
    m_button_selected->set_diffuse_color(storm::Color::from_hex(m_dwFaceColor));
}

void CXI_TEXTBUTTON::ReleaseAll()
{
    PICTURE_TEXTURE_RELEASE(pPictureService, m_sGroupName, m_idTex);

    m_button.reset();
    m_button_selected.reset();

    m_shadow.reset();
    m_selection.reset();
    m_back.reset();

    m_font.reset();
    m_text.reset();

    delete[] m_sGroupName;
    m_sGroupName = nullptr;

    delete[] m_sString;
    m_sString = nullptr;
}

int CXI_TEXTBUTTON::CommandExecute(int wActCode)
{
    if (m_bUse) {
        switch (wActCode) {
        case ACTION_MOUSECLICK:
        case ACTION_ACTIVATE:
        case ACTION_MOUSEDBLCLICK:
            if (m_bMakeActionInDeclick)
                m_nPressedDelay = 0;
            else
                m_nPressedDelay = m_nMaxDelay;
            break;
        }
    }
    return -1;
}

bool CXI_TEXTBUTTON::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom && m_bClickable && m_bUse) return true;

    return false;
}

void CXI_TEXTBUTTON::ChangePosition(XYRECT& rNewPos)
{
    m_rect = rNewPos;

    m_rect_pressed = {
        .left   = m_rect.left + m_fXDeltaPress,
        .top    = m_rect.top + m_fYDeltaPress,
        .right  = m_rect.right + m_fXDeltaPress,
        .bottom = m_rect.bottom + m_fYDeltaPress,
    };

    if (m_shadow) {
        auto const shadow_h_offset = (m_rect.right - m_rect.left) * (m_fShadowScale - 1.F) * .5f;
        auto const shadow_v_offset = (m_rect.bottom - m_rect.top) * (m_fShadowScale - 1.F) * .5f;

        m_shadow_rect = storm::FRect {
            .left   = m_rect.left - shadow_h_offset + m_fXShadow,
            .top    = m_rect.top - shadow_v_offset + m_fYShadow,
            .right  = m_rect.right + shadow_h_offset + m_fXShadow,
            .bottom = m_rect.bottom + shadow_v_offset + m_fYShadow,
        };

        m_shadow_rect_pressed = {
            .left   = m_shadow_rect.left + m_fXDeltaPress,
            .top    = m_shadow_rect.top + m_fYDeltaPress,
            .right  = m_shadow_rect.right + m_fXDeltaPress,
            .bottom = m_shadow_rect.bottom + m_fYDeltaPress,
        };
    }
}

void CXI_TEXTBUTTON::SaveParametersToIni()
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

uint32_t CXI_TEXTBUTTON::MessageProc(int32_t msgcode, MESSAGE& message)
{
    switch (msgcode) {
    case 0:  // change the text on the button
    {
        std::string const& param = message.String();
        STORM_DELETE(m_sString);
        m_idString = -1;
        if (param[0] == '#') {
            {
                auto const len = param.size();
                if ((m_sString = new char[len]) == nullptr) { throw std::runtime_error("allocate memory error"); }
                memcpy(m_sString, param.c_str() + 1, len);
            }
        } else if (core->GetTargetEngineVersion() <= storm::ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN) {
            auto const len = param.size();
            if ((m_sString = new char[len + 1]) == nullptr) { throw std::runtime_error("allocate memory error"); }
            memcpy(m_sString, param.c_str(), len + 1);
        } else {
            m_idString = pStringService->GetStringNum(param.c_str());
        }
    } break;
    case 1:  // change button position
        XYRECT newPos;
        newPos.left   = message.Long();
        newPos.top    = message.Long();
        newPos.right  = message.Long();
        newPos.bottom = message.Long();
        ChangePosition(newPos);
        break;
    }
    return 0;
}

void CXI_TEXTBUTTON::SetUsing(bool bUsing)
{
    m_bUse          = bUsing;
    m_nPressedDelay = 0;
}

void CXI_TEXTBUTTON::MakeLClickPreaction()
{
    auto const mouse_pos = ptrOwner->GetMousePoint();
    if (mouse_pos.x >= m_rect.left && mouse_pos.x <= m_rect.right && mouse_pos.y >= m_rect.top && mouse_pos.y <= m_rect.bottom) {
        m_nPressedDelay = m_nMaxDelay;
    }
}
