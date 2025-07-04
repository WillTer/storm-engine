#include "xi_video.h"

#include <libs/renderer_next/ui/texture_sequence.h>

CXI_VIDEO::CXI_VIDEO() : m_dwColor(0)
{
    m_nNodeType = NODETYPE_VIDEO;
    m_video_tex = nullptr;
}

CXI_VIDEO::~CXI_VIDEO()
{
    ReleaseAll();
}

uint32_t vid_counter = 0;

void CXI_VIDEO::pre_draw(storm::GPUCommandBuffer const& cmd_buffer, uint32_t delta_time)
{
    if (m_video_tex) { m_video_tex->pre_draw(cmd_buffer, delta_time); }
}

void CXI_VIDEO::update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time)
{
    if (!m_video_tex && !m_video_tex_name.empty()) {
        m_video_tex = std::make_unique<storm::TextureSequence>(copy_pass, m_video_tex_name);
        m_video_tex->set_rect(m_rect);
        m_video_tex->set_screen_rect(m_screen_rect);
        m_video_tex->set_diffuse_color(storm::Color::from_hex(m_dwColor) * 2);
    }

    if (m_video_tex) { m_video_tex->update(copy_pass, delta_time); }
}

void CXI_VIDEO::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse && m_video_tex) { m_video_tex->draw(render_pass); }
}

bool CXI_VIDEO::Init(
    INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
{
    if (!CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize)) return false;
    SetGlowCursor(false);
    return true;
}

void CXI_VIDEO::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    char param[255];

    if (ReadIniString(ini1, name1, ini2, name2, "sTexture", param, sizeof(param), "")) { m_video_tex_name = param; }

    m_rectTex = GetIniFloatRect(ini1, name1, ini2, name2, "textureRect", FXYRECT(0.f, 0.f, 1.f, 1.f));

    m_dwColor = GetIniARGB(ini1, name1, ini2, name2, "color", 0xFFFFFFFF);
}

void CXI_VIDEO::ReleaseAll()
{
    m_video_tex.reset();
}

int CXI_VIDEO::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_VIDEO::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return false;
}

void CXI_VIDEO::ChangePosition(XYRECT& rNewPos)
{
    m_rect = rNewPos;
    m_video_tex->set_rect(m_rect);
}

void CXI_VIDEO::SaveParametersToIni()
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
