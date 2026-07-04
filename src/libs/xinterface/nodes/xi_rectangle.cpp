#include "xi_rectangle.h"

#include <libs/renderer_next/types.h>
#include <libs/renderer_next/ui/rectangle.h>

CXI_RECTANGLE::CXI_RECTANGLE()
{
    m_nNodeType = NODETYPE_RECTANGLE;
}

CXI_RECTANGLE::~CXI_RECTANGLE()
{
    ReleaseAll();
}

void CXI_RECTANGLE::update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time)
{
    m_back->set_screen_rect(m_screen_rect);
    m_back->update(copy_pass, delta_time);

    if (m_border) {
        m_border->set_screen_rect(m_screen_rect);
        m_border->update(copy_pass, delta_time);
    }
}

void CXI_RECTANGLE::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse) {
        m_back->draw(render_pass);

        if (m_border) {
            m_border->draw(render_pass);
        }
    }
}

bool CXI_RECTANGLE::Init(
    INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
{
    return CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize);
}

void CXI_RECTANGLE::ReleaseAll() {}

int CXI_RECTANGLE::CommandExecute(int wActCode)
{
    return -1;
}

void CXI_RECTANGLE::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    // Get rectangle left colors
    m_dwLeftColor = GetIniARGB(ini1, name1, ini2, name2, "leftColor", 0);

    // Get rectangle top colors
    m_dwTopColor = GetIniARGB(ini1, name1, ini2, name2, "topColor", 0);

    // Get rectangle right colors
    m_dwRightColor = GetIniARGB(ini1, name1, ini2, name2, "rightColor", 0);

    // Get rectangle bottom colors
    m_dwBottomColor = GetIniARGB(ini1, name1, ini2, name2, "bottomColor", 0);

    // Get bounder parameters
    uint32_t const border_color = GetIniARGB(ini1, name1, ini2, name2, "borderColor", 0);
    create_border(border_color);

    m_back = std::make_unique<storm::Rectangle>(storm::Color::from_hex(m_dwLeftColor));
    m_back->set_rect(m_rect);
    m_back->set_screen_rect(m_screen_rect);

    UpdateColors();
}

void CXI_RECTANGLE::UpdateColors()
{
    // set left top vertex color
    uint8_t alpha = (ALPHA(m_dwLeftColor) * ALPHA(m_dwTopColor)) >> 8L;
    uint8_t red   = (RED(m_dwLeftColor) * RED(m_dwTopColor)) >> 8L;
    uint8_t green = (GREEN(m_dwLeftColor) * GREEN(m_dwTopColor)) >> 8L;
    uint8_t blue  = (BLUE(m_dwLeftColor) * BLUE(m_dwTopColor)) >> 8L;
    m_back->set_vertex_color(0, {alpha, red, green, blue});

    // set left bottom vertex color
    alpha = (ALPHA(m_dwLeftColor) * ALPHA(m_dwBottomColor)) >> 8L;
    red   = (RED(m_dwLeftColor) * RED(m_dwBottomColor)) >> 8L;
    green = (GREEN(m_dwLeftColor) * GREEN(m_dwBottomColor)) >> 8L;
    blue  = (BLUE(m_dwLeftColor) * BLUE(m_dwBottomColor)) >> 8L;
    m_back->set_vertex_color(1, {alpha, red, green, blue});

    // set right top vertex color
    alpha = (ALPHA(m_dwRightColor) * ALPHA(m_dwTopColor)) >> 8L;
    red   = (RED(m_dwRightColor) * RED(m_dwTopColor)) >> 8L;
    green = (GREEN(m_dwRightColor) * GREEN(m_dwTopColor)) >> 8L;
    blue  = (BLUE(m_dwRightColor) * BLUE(m_dwTopColor)) >> 8L;
    m_back->set_vertex_color(2, {alpha, red, green, blue});

    // set right bottom vertex color
    alpha = (ALPHA(m_dwRightColor) * ALPHA(m_dwBottomColor)) >> 8L;
    red   = (RED(m_dwRightColor) * RED(m_dwBottomColor)) >> 8L;
    green = (GREEN(m_dwRightColor) * GREEN(m_dwBottomColor)) >> 8L;
    blue  = (BLUE(m_dwRightColor) * BLUE(m_dwBottomColor)) >> 8L;
    m_back->set_vertex_color(3, {alpha, red, green, blue});
}

void CXI_RECTANGLE::create_border(uint32_t color)
{
    if (ALPHA(color) == 0) {
        return;
    }

    if (m_border) {
        m_border->set_color(storm::Color::from_hex(color));
        return;
    }

    m_border = std::make_unique<storm::Rectangle>(storm::Color::from_hex(color), storm::Rectangle::Fill::None);
    m_border->set_rect(m_rect);
    m_border->set_screen_rect(m_screen_rect);
}

bool CXI_RECTANGLE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (!m_bClickable) {
        return false;
    }

    if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom) {
        return true;
    }

    return false;
}

void CXI_RECTANGLE::ChangePosition(XYRECT& rNewPos)
{
    m_rect = rNewPos;
    m_back->set_rect(m_rect);
}

void CXI_RECTANGLE::SaveParametersToIni()
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

uint32_t CXI_RECTANGLE::MessageProc(int32_t msgcode, MESSAGE& message)
{
    switch (msgcode) {
    case 0:  // Change the position of the rectangle
    {
        XYRECT new_rect = {};
        new_rect.left   = message.Long();
        new_rect.top    = message.Long();
        new_rect.right  = message.Long();
        new_rect.bottom = message.Long();
        ChangePosition(new_rect);
    } break;
    case 1:  // Change rectangle and border color
    {
        m_dwTopColor = m_dwBottomColor = storm::Color {255, 255, 255, 255}.to_hex();
        m_dwLeftColor = m_dwRightColor = message.Long();
        uint32_t const border_color    = message.Long();
        create_border(border_color);
        UpdateColors();
    } break;
    }

    return 0;
}
