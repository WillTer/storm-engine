#include "xi_picture.h"

#include <libs/filesystem/default_paths.h>
#include <libs/filesystem/v_file_service.h>
#include <libs/renderer_next/types.h>
#include <libs/renderer_next/ui/image_2d.h>
#include <libs/util/storm_assert.h>
#include <libs/util/string_compare.hpp>

namespace
{
std::string const TECHNIQUE_NAME = "iVideo";
}

CXI_PICTURE::CXI_PICTURE()
{
    m_nNodeType       = NODETYPE_PICTURE;
    m_pcGroupName     = nullptr;
    m_bMakeBlind      = false;
    m_fCurBlindTime   = 0.f;
    m_bBlindUp        = true;
    m_fBlindUpSpeed   = 0.001f;
    m_fBlindDownSpeed = 0.001f;
    m_dwBlindMin      = storm::Color {255, 128, 128, 128}.to_hex();
    m_dwBlindMax      = storm::Color {255, 255, 255, 255}.to_hex();
    m_picture         = nullptr;
}

CXI_PICTURE::~CXI_PICTURE()
{
    ReleaseAll();
}

void CXI_PICTURE::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    if (m_bUse && m_picture) {
        m_picture->set_rect(m_rect);
        m_picture->set_screen_rect(m_screen_rect);
        m_picture->set_diffuse_color(m_picture_color);
        m_picture->draw(render_pass);
    }
}

bool CXI_PICTURE::Init(
    INIFILE*             ini1,
    char const*          name1,
    INIFILE*             ini2,
    char const*          name2,
    /*VDX9RENDER*/ void* rs,
    XYRECT&              hostRect,
    XYPOINT&             ScreenSize)
{
    return CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize);
}

void CXI_PICTURE::update(storm::GPUCopyPass const& copy_pass, uint32_t delta_time)
{
    if (m_bMakeBlind) {
        if (m_bBlindUp) {
            m_fCurBlindTime += m_fBlindUpSpeed * delta_time;
            if (m_fCurBlindTime >= 1.F) {
                m_fCurBlindTime = 1.F;
                m_bBlindUp      = false;
            }
        } else {
            m_fCurBlindTime -= m_fBlindDownSpeed * delta_time;
            if (m_fCurBlindTime <= 0.F) {
                m_fCurBlindTime = 0.F;
                m_bBlindUp      = true;
            }
        }

        ChangeColor(ptrOwner->GetBlendColor(m_dwBlindMin, m_dwBlindMax, m_fCurBlindTime));
    }

    if (m_picture) {
        m_picture->update(copy_pass, delta_time);
    }

    ChangePosition(m_rect);
}

void CXI_PICTURE::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    char param[255];

    if (ReadIniString(ini1, name1, ini2, name2, "groupName", param, sizeof(param), "")) {
        auto const len = strlen(param) + 1;
        m_pcGroupName  = new char[len];
        Assert(m_pcGroupName);
        memcpy(m_pcGroupName, param, len);
        auto const texture = pPictureService->get_texture(m_pcGroupName);

        m_picture = std::make_unique<storm::Image2D>(texture, TECHNIQUE_NAME);
        if (ReadIniString(ini1, name1, ini2, name2, "picName", param, sizeof(param), "")) {
            m_picture->set_uv(pPictureService->get_texture_uv(m_pcGroupName, param));
        }
    } else if (ReadIniString(ini1, name1, ini2, name2, "textureName", param, sizeof(param), "")) {
        m_picture = std::make_unique<storm::Image2D>(param, TECHNIQUE_NAME);

        auto const tex_rect = GetIniFloatRect(ini1, name1, ini2, name2, "textureRect", FXYRECT(0.F, 0.F, 1.F, 1.F));
        m_picture->set_uv(tex_rect);
    } else if (ReadIniString(ini1, name1, ini2, name2, "videoName", param, sizeof(param), "")) {
        m_picture = std::make_unique<storm::Image2D>(pPictureService->get_video_texture(param), TECHNIQUE_NAME);
    }

    assert(m_picture);

    auto const picture_color =
        storm::Color::from_hex(GetIniARGB(ini1, name1, ini2, name2, "color", storm::Color {255, 128, 128, 128}.to_hex()));
    ChangeColor(picture_color);

    // Create rectangle
    ChangePosition(m_rect);

    m_bMakeBlind    = GetIniBool(ini1, name1, ini2, name2, "blind", false);
    m_fCurBlindTime = 0.F;
    m_bBlindUp      = true;
    auto tmp        = GetIniFloat(ini1, name1, ini2, name2, "blindUpTime", 1.F);
    if (tmp > 0.F) {
        m_fBlindUpSpeed = 0.001F / tmp;
    }
    tmp = GetIniFloat(ini1, name1, ini2, name2, "blindDownTime", 1.F);
    if (tmp > 0.F) {
        m_fBlindDownSpeed = 0.001F / tmp;
    }
    m_dwBlindMin = GetIniARGB(ini1, name1, ini2, name2, "blindMinColor", storm::Color {255, 128, 128, 128}.to_hex());
    m_dwBlindMax = GetIniARGB(ini1, name1, ini2, name2, "blindMaxColor", storm::Color {255, 255, 255, 255}.to_hex());
}

void CXI_PICTURE::ReleaseAll()
{
    ReleasePicture();
}

int CXI_PICTURE::CommandExecute(int wActCode)
{
    return -1;
}

bool CXI_PICTURE::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    if (m_bClickable) {
        if (xPos >= m_rect.left && xPos <= m_rect.right && yPos >= m_rect.top && yPos <= m_rect.bottom) return true;
    }
    return false;
}

void CXI_PICTURE::ChangePosition(XYRECT& rNewPos)
{
    m_rect = rNewPos;
}

void CXI_PICTURE::SaveParametersToIni()
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

void CXI_PICTURE::SetNewPicture(bool video, char const* sNewTexName)
{
    ReleasePicture();
    if (video) {
        m_picture = std::make_unique<storm::Image2D>(pPictureService->get_video_texture(sNewTexName), TECHNIQUE_NAME);
    } else {
        m_picture = std::make_unique<storm::Image2D>(sNewTexName, TECHNIQUE_NAME);
    }
}

void CXI_PICTURE::SetNewPictureFromDir(char const* dirName)
{
    auto const path      = fio->base_directory_path(BaseDirectory::Textures) / dirName;
    auto const filenames = fio->string_paths_by_mask(path, "*.tx", false);
    if (!filenames.empty()) {
        char param[512];
        int  findQ = rand() % filenames.size();
        sprintf(param, "%s/%s", dirName, filenames[findQ].c_str());
        int const paramlen = strlen(param);
        if (paramlen < sizeof(param) && paramlen >= 3) {
            param[paramlen - 3] = 0;
        }
        SetNewPicture(false, param);
    }
}

void CXI_PICTURE::SetNewPictureByGroup(char const* groupName, char const* picName)
{
    if (!m_pcGroupName || !storm::iEquals(m_pcGroupName, groupName)) {
        ReleasePicture();
        if (groupName) {
            auto const len = strlen(groupName) + 1;
            m_pcGroupName  = new char[strlen(groupName) + 1];
            Assert(m_pcGroupName);
            memcpy(m_pcGroupName, groupName, len);
        }
    }

    m_picture = std::make_unique<storm::Image2D>(pPictureService->get_texture(m_pcGroupName), TECHNIQUE_NAME);
    m_picture->set_uv(pPictureService->get_texture_uv(m_pcGroupName, picName));
}

uint32_t CXI_PICTURE::MessageProc(int32_t msgcode, MESSAGE& message)
{
    switch (msgcode) {
    case 0:  // Move the picture to a new position
    {
        m_rect.left   = message.Long();
        m_rect.top    = message.Long();
        m_rect.right  = message.Long();
        m_rect.bottom = message.Long();
        ChangePosition(m_rect);
    } break;

    case 1:  // Set the texture coordinates of the image
    {
        FXYRECT tex_rect = {};
        tex_rect.left    = message.Float();
        tex_rect.right   = message.Float();
        tex_rect.top     = message.Float();
        tex_rect.bottom  = message.Float();
        ChangeUV(tex_rect);
    } break;

    case 2:  // Set a new picture or video picture
    {
        auto const         video = message.Long() != 0;
        std::string const& param = message.String();
        SetNewPicture(video, param.c_str());
    } break;

    case 3:  // Get a random picture from the directory
    {
        std::string const& param = message.String();
        SetNewPictureFromDir(param.c_str());
    } break;

    case 4:  // Set a new color
    {
        uint32_t const color = message.Long();
        ChangeColor(color);
    } break;

    case 5:  // set / remove blinking
    {
        bool const is_blind = message.Long() != 0;
        if (m_bMakeBlind != is_blind) {
            m_bMakeBlind = is_blind;
            if (!m_bMakeBlind) {
                ChangeColor(m_dwBlindMin);
            } else {
                m_fCurBlindTime = 0.f;
                m_bBlindUp      = true;
            }
        }
    } break;

    case 6:  // set new picture by group and picture name
    {
        std::string const& group_name = message.String();
        std::string const& pic_name   = message.String();
        SetNewPictureByGroup(group_name.c_str(), pic_name.c_str());
    } break;

    case 7:  // set new picture by pointer to IDirect3DTexture9
    {
        int32_t pTex = -1;
        if (message.GetCurrentFormatType() == 'p') {
            // DEPRECATED
            core->Trace("Warning! Setting an interface picture by pointer is deprecated. Please use integers instead.");
            pTex = message.Pointer();
        } else {
            pTex = message.Long();
        }
        SetNewPictureByPointer(pTex);
    } break;

    case 8:  // remove texture from other picture to this
    {
        std::string const& src_node_name = message.String();
        auto*              node          = static_cast<CINODE*>(ptrOwner->FindNode(src_node_name.c_str(), nullptr));
        if (node->m_nNodeType != NODETYPE_PICTURE) {
            core->Trace("Warning! XINTERFACE:: node with name %s have not picture type.", src_node_name.c_str());
        } else {
            ReleasePicture();
            auto* other_pic = static_cast<CXI_PICTURE*>(node);
            if (other_pic->m_pcGroupName != nullptr) {
                m_pcGroupName            = other_pic->m_pcGroupName;
                other_pic->m_pcGroupName = nullptr;
            }
            if (other_pic->m_picture != nullptr) {
                m_picture            = std::move(other_pic->m_picture);
                other_pic->m_picture = nullptr;
            }
            other_pic->ReleasePicture();
        }
    } break;
    }

    return 0;
}

void CXI_PICTURE::ChangeUV(FXYRECT& frNewUV)
{
    assert(false);
}

void CXI_PICTURE::ChangeColor(uint32_t dwColor)
{
    ChangeColor(storm::Color::from_hex(dwColor));
}

void CXI_PICTURE::ChangeColor(storm::Color const& color)
{
    m_picture_color = color;
}

void CXI_PICTURE::SetPictureSize(int32_t& nWidth, int32_t& nHeight)
{
    // if (!m_pTex && m_idTex == -1) {
    //     m_bUse = false;
    //     nWidth = nHeight = 0;
    //     return;
    // }

    if (nWidth <= 0) {
        // find the real width
        nWidth = 128;
    }
    if (nHeight <= 0) {
        // find the real height
        nHeight = 128;
    }

    if (nWidth < 0 || nHeight < 0) {
        m_bUse = false;
        nWidth = nHeight = 0;
        return;
    }

    XYRECT rNewPos = m_rect;
    if (rNewPos.right - rNewPos.left != nWidth) {
        rNewPos.left  = (m_rect.left + m_rect.right - nWidth) / 2;
        rNewPos.right = rNewPos.left + nWidth;
    }
    if (rNewPos.bottom - rNewPos.top != nHeight) {
        rNewPos.top    = (m_rect.top + m_rect.bottom - nHeight) / 2;
        rNewPos.bottom = rNewPos.top + nHeight;
    }
    ChangePosition(rNewPos);
}

void CXI_PICTURE::SetNewPictureByPointer(int32_t textureId)
{
    // Obsolete
}

void CXI_PICTURE::ReleasePicture()
{
    delete[] m_pcGroupName;
    m_pcGroupName = nullptr;
    m_picture.reset();
}
