#include "free_camera.h"

#include <libs/collide/collide.h>
#include <libs/core/save_load.h>

#define SENSITIVITY 0.0015f
#define FOV 1.285f

entid_t  sphere;
COLLIDE* pCollide;

FreeCamera::FreeCamera()
{
    SetOn(false);
    SetActive(false);

    pIslandBase = nullptr;
    vPos.z      = 250.0f;
    vPos.y      = 3.0f;
    fFov        = FOV;

    iLockX = 0;
    iLockY = 0;

    fCameraOnEarthHeight = 3.0f;
    bCameraOnEarth       = false;
}

FreeCamera::~FreeCamera() {}

bool FreeCamera::Init()
{
    SetDevice();
    return true;
}

void FreeCamera::SetDevice()
{
    pCollide = static_cast<COLLIDE*>(core->GetService("CollideService"));
    Assert(pCollide);
}

bool FreeCamera::CreateState(ENTITY_STATE_GEN* state_gen) const
{
    state_gen->SetState("vv", sizeof(vPos), vPos, sizeof(vAng), vAng);
    return true;
}

bool FreeCamera::LoadState(ENTITY_STATE* state)
{
    SetDevice();
    state->Struct(sizeof(vPos), (char*)&vPos);
    state->Struct(sizeof(vAng), (char*)&vAng);
    return true;
}

void FreeCamera::Execute(uint32_t Delta_Time)
{
    if (!isOn()) return;

    SetPerspective(AttributesPointer->GetAttributeAsFloat("Perspective"));

    float persp;
    // pRS->GetCamera(vPos, vAng, persp);

    if (!pIslandBase) pIslandBase = static_cast<ISLAND_BASE*>(core->GetEntityPointer(core->GetEntityId("Island")));

    Move(core->GetDeltaTime());
}

void FreeCamera::Move(uint32_t DeltaTime)
{
    if (!isActive()) return;

    // POINT pnt;
    // GetCursorPos(&pnt);
    // if(pnt.x != iLockX || pnt.y != iLockY)
    CONTROL_STATE cs;

    {
        core->Controls->GetControlState("FreeCamera_Turn_H", cs);
        vAng.y += SENSITIVITY * static_cast<float>(cs.fValue);
        core->Controls->GetControlState("FreeCamera_Turn_V", cs);
        vAng.x += SENSITIVITY * static_cast<float>(cs.fValue);
        // SetCursorPos(iLockX,iLockY);
    }
    if (bCameraOnEarth && pIslandBase) {
        pIslandBase->GetDepth(vPos.x, vPos.z, &vPos.y);
        vPos.y += fCameraOnEarthHeight;
    }
    auto const c0    = cosf(vAng.y);
    auto const s0    = sinf(vAng.y);
    auto const c1    = cosf(vAng.x);
    auto const s1    = sinf(vAng.x);
    auto       c2    = cosf(vAng.z);
    float      s2    = sinf(vAng.z);
    float      speed = 5.0f * 0.001f * static_cast<float>(DeltaTime);

    if (core->Controls->GetAsyncKeyState(VK_SHIFT)) speed *= 4.0f;
    if (core->Controls->GetAsyncKeyState(VK_CONTROL)) speed *= 8.0f;

    core->Controls->GetControlState("FreeCamera_Forward", cs);
    if (cs.state == CST_ACTIVE) vPos += speed * CVECTOR(s0 * c1, -s1, c0 * c1);
    core->Controls->GetControlState("FreeCamera_Backward", cs);
    if (cs.state == CST_ACTIVE) vPos -= speed * CVECTOR(s0 * c1, -s1, c0 * c1);

    // pRS->SetCamera(vPos, vAng, GetPerspective());
}

void FreeCamera::Save(CSaveLoad* pSL)
{
    pSL->SaveVector(vPos);
    pSL->SaveVector(vAng);
    pSL->SaveFloat(fFov);
    pSL->SaveLong(iLockX);
    pSL->SaveLong(iLockY);

    pSL->SaveDword(bCameraOnEarth);
    pSL->SaveFloat(fCameraOnEarthHeight);
}

void FreeCamera::Load(CSaveLoad* pSL)
{
    vPos   = pSL->LoadVector();
    vAng   = pSL->LoadVector();
    fFov   = pSL->LoadFloat();
    iLockX = pSL->LoadLong();
    iLockY = pSL->LoadLong();

    bCameraOnEarth       = pSL->LoadDword() != 0;
    fCameraOnEarthHeight = pSL->LoadFloat();
}
