#include "xi_line_collection.h"

#include <stdio.h>

CXI_LINECOLLECTION::CXI_LINECOLLECTION()
{
    m_nNodeType = NODETYPE_LINECOLLECTION;
}

CXI_LINECOLLECTION::~CXI_LINECOLLECTION()
{
    ReleaseAll();
}

int CXI_LINECOLLECTION::CommandExecute(int wActCode)
{
    return -1;
}

void CXI_LINECOLLECTION::Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time)
{
    // if (m_bUse) { m_rs->DrawLines(m_aLines.data(), m_aLines.size() / 2, "iLineCollection"); }
}

bool CXI_LINECOLLECTION::Init(
    INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
{
    if (!CINODE::Init(ini1, name1, ini2, name2, rs, hostRect, ScreenSize)) return false;
    // screen position for that is host screen position
    memcpy(&m_rect, &m_hostRect, sizeof(m_hostRect));
    SetGlowCursor(false);
    return true;
}

void CXI_LINECOLLECTION::LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2)
{
    char param[256];
    char param1[256];

    // fill lines structure array
    auto const bRelativeRect = !GetIniLong(ini1, name1, ini2, name2, "bAbsoluteRectangle", 0);
    auto       nCurLine      = 0;
    if (ini1->ReadString(name1, "line", param, sizeof(param) - 1, "")) do {
            XYRECT   scrRect;
            uint32_t dwCol = 0;
            if (GetMidStr(param, param1, sizeof(param1), "(", ")-(")) GetDataStr(param1, "ll", &scrRect.left, &scrRect.top);
            if (GetMidStr(param, param1, sizeof(param1), ")-(", ")")) GetDataStr(param1, "ll", &scrRect.right, &scrRect.bottom);
            if (GetMidStr(param, param1, sizeof(param1), "col:{", "}")) { dwCol = GetColorFromStr(param1, dwCol); }
            if (bRelativeRect) GetRelativeRect(scrRect);

            // m_aLines.push_back(RS_LINE {CVECTOR {static_cast<float>(scrRect.left), static_cast<float>(scrRect.top), 1.f}, dwCol});
            // m_aLines.push_back(RS_LINE {CVECTOR {static_cast<float>(scrRect.right), static_cast<float>(scrRect.bottom), 1.f}, dwCol});

            nCurLine++;
        } while (ini1->ReadStringNext(name1, "line", param, sizeof(param) - 1));
}

void CXI_LINECOLLECTION::ReleaseAll()
{
    // m_aLines.clear();
}

bool CXI_LINECOLLECTION::IsClick(int buttonID, int32_t xPos, int32_t yPos)
{
    return false;
}

void CXI_LINECOLLECTION::ChangePosition(XYRECT& rNewPos)
{
    // no this action
}

void CXI_LINECOLLECTION::SaveParametersToIni()
{
    //    char pcWriteParam[2048];

    auto pIni = fio->open_ini_file(ptrOwner->m_sDialogFileName.c_str());
    if (!pIni) {
        core->Trace("Warning! Can`t open ini file name %s", ptrOwner->m_sDialogFileName.c_str());
        return;
    }

    // save position
    //    sprintf_s( pcWriteParam, sizeof(pcWriteParam), "%d,%d,%d,%d", m_rect.left, m_rect.top, m_rect.right,
    // m_rect.bottom );     pIni->WriteString( m_nodeName, "position", pcWriteParam );
}

uint32_t CXI_LINECOLLECTION::MessageProc(int32_t msgcode, MESSAGE& message)
{
    switch (msgcode) {
    case 0:  // change color for line with number or all lines (if number = -1)
    {
        uint32_t const dwColor  = message.Long();
        auto const     nLineNum = message.Long();
        // if (nLineNum < 0 || nLineNum >= static_cast<int32_t>(m_aLines.size()) / 2) {
        //     for (int32_t n = 0; n < m_aLines.size(); n++)
        //         m_aLines[n].dwColor = dwColor;
        // } else {
        //     m_aLines[nLineNum * 2].dwColor = m_aLines[nLineNum * 2 + 1].dwColor = dwColor;
        // }
    } break;
    case 1:  // add line and return its number
    {
        uint32_t const dwColor = message.Long();
        auto const     nLeft   = message.Long();
        auto const     nTop    = message.Long();
        auto const     nRight  = message.Long();
        int32_t const  nBottom = message.Long();
        // int32_t const nLineNum = m_aLines.size() / 2;
        // m_aLines.resize(m_aLines.size() + 2);
        // m_aLines[nLineNum * 2].dwColor = m_aLines[nLineNum * 2 + 1].dwColor = dwColor;
        // m_aLines[nLineNum * 2].vPos.z = m_aLines[nLineNum * 2 + 1].vPos.z = 1.f;
        // m_aLines[nLineNum * 2].vPos.x                                     = static_cast<float>(nLeft);
        // m_aLines[nLineNum * 2 + 1].vPos.x                                 = static_cast<float>(nRight);
        // m_aLines[nLineNum * 2].vPos.y                                     = static_cast<float>(nTop);
        // m_aLines[nLineNum * 2 + 1].vPos.y                                 = static_cast<float>(nBottom);
        // return nLineNum;
    } break;
    }
    return 0;
}
