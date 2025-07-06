#include "sea_cameras.h"

#include <libs/shared_headers/sea_ai/script_defines.h>
#include <libs/window/os_window.hpp>

#include "deck_camera.h"
#include "free_camera.h"
#include "ship_camera.h"

SeaCameras::SeaCameras()
{
    bActive = true;
    core->GetWindow()->show_cursor(false);
}

SeaCameras::~SeaCameras()
{
    core->GetWindow()->show_cursor(true);
}

void SeaCameras::ProcessMessage(uint32_t iMsg, uint32_t wParam, uint32_t lParam) {}

uint64_t SeaCameras::ProcessMessage(MESSAGE& message)
{
    uint32_t i;
    switch (message.Long()) {
    case AI_CAMERAS_ADD_CAMERA: {
        auto const eidCamera = message.EntityID();
        auto*      pCamera   = static_cast<COMMON_CAMERA*>(core->GetEntityPointer(eidCamera));
        // if (CamerasArray.Find(pCamera) == INVALID_ARRAY_INDEX) CamerasArray.Add(pCamera);
        auto const it = std::find(CamerasArray.begin(), CamerasArray.end(), pCamera);
        if (it == CamerasArray.end()) CamerasArray.push_back(pCamera);
        pCamera->SetOn(false);
        pCamera->SetActive(bActive);
    } break;
    case AI_CAMERAS_SET_CAMERA: {
        auto const  eidCamera   = message.EntityID();
        auto* const pACharacter = message.AttributePointer();
        auto*       pCamera     = static_cast<COMMON_CAMERA*>(core->GetEntityPointer(eidCamera));
        // if (CamerasArray.Find(pCamera) == INVALID_ARRAY_INDEX) CamerasArray.Add(pCamera);
        auto const it = std::find(CamerasArray.begin(), CamerasArray.end(), pCamera);
        if (it == CamerasArray.end()) CamerasArray.push_back(pCamera);
        for (i = 0; i < CamerasArray.size(); i++)
            CamerasArray[i]->SetOn(false);
        pCamera->SetOn(true);
        pCamera->SetActive(bActive);
        pCamera->SetCharacter(pACharacter);
    } break;
    case AI_MESSAGE_SEASAVE: {
        auto* pSL = (CSaveLoad*)message.Pointer();
        for (i = 0; i < CamerasArray.size(); i++)
            CamerasArray[i]->Save(pSL);
    } break;
    case AI_MESSAGE_SEALOAD: {
        auto* pSL = (CSaveLoad*)message.Pointer();
        for (i = 0; i < CamerasArray.size(); i++)
            CamerasArray[i]->Load(pSL);
    } break;
    }
    return 0;
}
