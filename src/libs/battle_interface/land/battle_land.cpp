#include "battle_land.h"

#include <libs/core/core.h>
#include <libs/core/message.h>
#include <libs/shared_headers/battle_interface/msg_control.h>
#include <libs/util/string_compare.hpp>

#include "battle_man_sign.h"

BattleLandInterface::BattleLandInterface() : m_bShowCommandos(false)
{
    m_pRS      = nullptr;
    m_pManSign = nullptr;
}

BattleLandInterface::~BattleLandInterface()
{
    Release();
}

bool BattleLandInterface::Init()
{
    m_pRS = static_cast<VDX9RENDER*>(core->GetService("RendererService"));
    if (!m_pRS) { throw std::runtime_error("Can`t create render service"); }

    SetShowParameters();
    return true;
}

void BattleLandInterface::Execute(uint32_t delta_time) const
{
    CONTROL_STATE cs;

    if (m_bShowCommandos && m_pManSign) {
        if (!m_pManSign->IsActive()) {
            core->Controls->GetControlState(BI_COMMANDS_ACTIVATE_LAND, cs);
            if (cs.state == CST_ACTIVATED) { m_pManSign->SetActive(true); }
        } else {
            m_pManSign->MakeControl();
        }
    }
}

void BattleLandInterface::Realize(uint32_t delta_time)
{
    if (m_bShowCommandos) {
        m_pRS->MakePostProcess();
        if (m_pManSign) m_pManSign->Draw();

        m_Images.Draw();
        BIUtils::PrintTextInfoArray(m_TextInfo);
    }
}

uint64_t BattleLandInterface::ProcessMessage(MESSAGE& message)
{
    switch (message.Long()) {
    case MSG_BATTLE_LAND_START:
        if (m_pManSign) m_pManSign->SetUpdate();
        break;

    case MSG_BATTLE_LAND_END: EndShow(); break;

    case BI_IN_SET_COMMAND_MODE:
        if (m_bShowCommandos && m_pManSign) {
            auto comMode            = message.Long();
            auto startTextureNumber = message.Long();
            auto startPictureNumber = message.Long();
            auto characterNum       = message.Long();
        }
        break;

    case MSG_BATTLE_LAND_SET_SHOW: m_bShowCommandos = (message.Long() != 0); break;

    case MSG_BATTLE_LAND_SET_MSGICONS: EnableMessageIcons(message.ScriptVariablePointer()); break;

    case BI_MSG_ADD_NEWTEXTURE: {
        std::string const& param = message.String();
        int const          hQ    = message.Long();
        int const          vQ    = message.Long();
        if (m_pManSign) return m_pManSign->AddTexture(param.c_str(), hQ, vQ);
        return -1;
    } break;

    case MSG_BATTLE_LAND_MAKE_COMMAND: {
        std::string const& param = message.String();
        if (storm::iEquals(param, "cancel")) {
            if (m_pManSign) m_pManSign->ExecuteCommand(BI_MSG_COMMAND_DEACTIVATE);
        }
    } break;

    case BI_MSG_CARE_COMMANDLIST_UPDATE: m_pManSign->SetUpdate(); break;
    }
    return 0;
}

void BattleLandInterface::SetParameters()
{
    UpdateCommandos();
    UpdateAlarm();
}

void BattleLandInterface::Release()
{
    m_pRS = nullptr;
    STORM_DELETE(m_pManSign);

    m_TextInfo.clear();
}

void BattleLandInterface::EndShow() {}

void BattleLandInterface::SetShowParameters()
{
    auto* const pA   = AttributesPointer ? AttributesPointer->GetAttributeClass("Parameters") : nullptr;
    m_bShowCommandos = 0 != BIUtils::GetIntFromAttr(pA, "DoShowCommandos", true);

    m_pManSign = new BIManSign(GetId(), m_pRS);
    Assert(m_pManSign);
    m_pManSign->Init(AttributesPointer, (AttributesPointer ? AttributesPointer->GetAttributeClass("ManSign") : nullptr));

    BIUtils::FillTextInfoArray(m_pRS, AttributesPointer ? AttributesPointer->GetAttributeClass("textinfo") : nullptr, m_TextInfo);

    m_Images.Init(m_pRS, AttributesPointer ? AttributesPointer->GetAttributeClass("imageslist") : nullptr);
}

void BattleLandInterface::UpdateCommandos() const
{
    auto* pA = AttributesPointer ? AttributesPointer->GetAttributeClass("data") : nullptr;
    if (pA) pA = pA->GetAttributeClass("icons");
    if (!pA) return;
}

void BattleLandInterface::UpdateAlarm() {}

void BattleLandInterface::EnableMessageIcons(VDATA* pvdat) {}

void BattleLandInterface::DisableMessageIcons() {}

void BattleLandInterface::SetTextData() {}
