#include "ai_cannon.h"

#include <libs/math/math_inlines.h>
#include <libs/shared_headers/sea_ai/script_defines.h>

uint64_t dwTmpRDTSC;

AICannon::AICannon() : pAHolder(nullptr), eidParent(0), vPos(), vDir(), fSpeedV0(0), vEnemyPos()
{
    bLoad             = false;
    bEmpty            = true;
    bFired            = false;
    fDamaged          = 0.0f;
    bRecharged        = true;
    bReady2Fire       = false;
    fTime2Action      = 0.0f;
    bCanRecharge      = true;
    fTotalTime2Action = 0.0f;
    CannonType        = CANNONTYPE_NORMAL;

    fMaxFireDistance = 100.0f;
}

AICannon::~AICannon() {}

void AICannon::Execute(float fDeltaTime)
{
    if (fTime2Action >= 0.0f) {
        if (isRecharged() && (!bCanRecharge || isEmpty()))
            ;
        else
            fTime2Action -= fDeltaTime;
    }

    if (fTime2Action <= 0.0f && isRecharged()) {
        bRecharged = false;
        if (!isEmpty()) bReady2Fire = true;
        return;
    }

    if (fTime2Action <= 0.0f && isFired() && !isDamaged() && !isEmpty()) {
        RDTSC_B(dwTmpRDTSC);
        RealFire();
        RDTSC_E(dwTmpRDTSC);
    }
}

float AICannon::CalcHeightFireAngle(float _fSpeedV0, const CVECTOR& vOur, const CVECTOR& vEnemy) const
{
    auto vTemp             = vOur - vEnemy;
    vTemp.y                = 0.0f;
    double const fDistance = sqrtf(~vTemp);
    auto const   g         = static_cast<double>(AIHelper::fGravity);
    double const fHeight   = (vEnemy.y - vOur.y);
    double const a         = SQR(g) / 4.0f;
    double const b         = (fHeight * g - SQR(_fSpeedV0));
    double const c         = SQR(fHeight) + SQR(fDistance);
    double const d         = b * b - 4 * a * c;
    if (d < 0) return 0.0f;

    double const x1 = (-b + sqrt(d)) / (2.0 * a);
    double const x2 = (-b - sqrt(d)) / (2.0 * a);

    double const t1 = sqrt(x1);
    double const t2 = sqrt(x2);

    auto const T = (CannonType == CANNONTYPE_MORTAR) ? t1 : t2;

    auto const fTemp  = fDistance / (_fSpeedV0 * T);
    auto const fAlpha = acos(Clamp(fTemp));

    auto const sy0 = _fSpeedV0 * T * sin(fAlpha) - g * SQR(T) / 2.0;
    auto const sy1 = _fSpeedV0 * T * sin(-fAlpha) - g * SQR(T) / 2.0;
    return (fabs(sy0 - fHeight) < fabs(sy1 - fHeight)) ? static_cast<float>(fAlpha) : static_cast<float>(-fAlpha);
}

VAI_OBJBASE* AICannon::GetAIObjPointer() const
{
    return static_cast<VAI_OBJBASE*>(core.GetEntityPointer(GetParentEID()));
}

float AICannon::GetDirY() const
{
    CMatrix mRot;
    auto*   pAIObj = GetAIObjPointer();

    (*pAIObj->GetMatrix()).Get3X3(mRot);

    auto const vDirTemp = mRot * vDir;
    return NormalizeAngle(atan2f(vDirTemp.x, vDirTemp.z));
}

CVECTOR AICannon::GetDir() const
{
    CMatrix mRot;
    auto*   pAIObj = GetAIObjPointer();
    (*pAIObj->GetMatrix()).Get3X3(mRot);
    return mRot * vDir;
}

CVECTOR AICannon::GetPos() const
{
    return *GetAIObjPointer()->GetMatrix() * vPos;
}

void AICannon::RealFire()
{
    CMatrix mRot;
    // calculate real world cannon position and direction,
    // calculate fire height and azimuth angle, and call script
    auto*      pAIObj   = GetAIObjPointer();
    auto const vPosTemp = *pAIObj->GetMatrix() * vPos;
    (*pAIObj->GetMatrix()).Get3X3(mRot);

    auto const vEnemyDir        = !(vEnemyPos - vPosTemp);
    auto const fFireDirection   = NormalizeAngle(atan2f(vEnemyDir.x, vEnemyDir.z));
    auto const fFireHeightAngle = CalcHeightFireAngle(fSpeedV0, vPosTemp, vEnemyPos);

    auto const vDirTemp = mRot * vDir;
    auto const fDirY    = NormalizeAngle(atan2f(vDirTemp.x, vDirTemp.z));

    // raw angle
    const CVECTOR vFlatEnemyDir = !(CVECTOR(vEnemyPos.x, vPosTemp.y, vEnemyPos.z) - vPosTemp);
    float         fAngle;
    float const   dp = vFlatEnemyDir | vEnemyDir;
    if (dp >= 1.0f) {
        fAngle = 0.0f;
    } else if (dp <= -1.0f) {
        fAngle = PI;
    } else {
        fAngle = acosf(dp);
    }
    if (vEnemyPos.y < vPosTemp.y) { fAngle = -fAngle; }

    core.Event(
        CANNON_FIRE,
        "afffffffff",
        pAHolder->GetACharacter(),
        vPosTemp.x,
        vPosTemp.y,
        vPosTemp.z,
        fSpeedV0,
        fFireDirection,
        fFireHeightAngle,
        fDirY,
        fMaxFireDistance,
        fAngle);

    pAIObj->Fire(*pAIObj->GetMatrix() * (vPos + vDir));

    bFired = false;
    bEmpty = true;
    Recharge();
}

bool AICannon::Fire(float _fSpeedV0, const CVECTOR& _vFirePos)
{
    if (isFired() || isRecharged() || isDamaged() || isEmpty()) return false;

    auto*      pAIObj   = GetAIObjPointer();
    auto const vPosTemp = *pAIObj->GetMatrix() * vPos;
    CMatrix    mRot;
    (*pAIObj->GetMatrix()).Get3X3(mRot);

    auto const vFireDir = !(_vFirePos - vPosTemp);
    auto const vDirTemp = mRot * vDir;
    auto const fDot     = (vFireDir | vDirTemp);
    if (fDot < 0.3f) return false;

    bFired      = true;
    bReady2Fire = false;

    vEnemyPos = _vFirePos;
    fSpeedV0  = _fSpeedV0;

    // calculate in script timeout before real fire
    auto* pVData = core.Event(CANNON_GET_FIRE_TIME, "a", pAHolder->GetACharacter());
    Assert(pVData);
    fTotalTime2Action = fTime2Action = pVData->GetFloat();
    return true;
}

void AICannon::Unload()
{
    // call script
    if (!isDamaged() && !isEmpty()) { core.Event(CANNON_UNLOAD, "a", pAHolder->GetACharacter()); }
    bReady2Fire = false;
    bRecharged  = false;
    bEmpty      = true;
    bFired      = false;
}

void AICannon::Load()
{
    auto* pVData = core.Event(CANNON_LOAD, "a", pAHolder->GetACharacter());
    Assert(pVData);
    bEmpty = pVData->GetInt() == 0;
}

void AICannon::Recharge()
{
    if (isFired() || isDamaged()) return;
    if (!isEmpty()) Unload();
    if (bCanRecharge) Load();
    bReady2Fire = false;
    bRecharged  = true;

    // calculate in script recharge time, and possibility of recharge
    auto* pVData = core.Event(CANNON_GET_RECHARGE_TIME, "a", pAHolder->GetACharacter());
    Assert(pVData);
    fTotalTime2Action = fTime2Action = pVData->GetFloat();
}

void AICannon::QuickRecharge(bool bHaveEnoughBalls)
{
    Recharge();
    fTime2Action = (bHaveEnoughBalls) ? 0.0f : fTotalTime2Action;
}

float AICannon::GetRechargePercent()
{
    return (isRecharged()) ? 1.0f - fTime2Action / fTotalTime2Action : 1.0f;
}

float AICannon::CalcMaxFireDistance(float fFireHeight, float fSpeedV0, float fAngle)
{
    auto const a  = -AIHelper::fGravity / 2.0f;
    auto const b  = fSpeedV0 * sinf(fAngle);
    auto const c  = fFireHeight;
    auto const d  = b * b - 4.0f * a * c;
    auto const t1 = (-b - sqrtf(d)) / (2.0f * a);
    // float t2 = (-b + sqrtf(d)) / (2.0f * a);

    auto const fDistance = fSpeedV0 * t1 * cosf(fAngle);
    // float s2 = fSpeedV0 * t2 * cosf(fAngle);

    return fDistance;
}

void AICannon::Init(AIAttributesHolder* _pAHolder, entid_t const eid, GEOS::LABEL& lbl)
{
    pAHolder  = _pAHolder;
    eidParent = eid;

    CMatrix m;
    memcpy(&m.m[0][0], &lbl.m[0][0], sizeof(m));

    vPos = m.Pos();
    vDir = CVECTOR(m.Vz().x, 0.0f, m.Vz().z);
    // vDir = atan2f(m.Vz().x,m.Vz().z);
}

void AICannon::Save(CSaveLoad* pSL) const
{
    pSL->SaveVector(vPos);
    pSL->SaveVector(vDir);
    pSL->SaveFloat(fTime2Action);
    pSL->SaveFloat(fTotalTime2Action);
    pSL->SaveFloat(fSpeedV0);
    pSL->SaveVector(vEnemyPos);
    pSL->SaveDword(bEmpty);
    pSL->SaveFloat(fDamaged);
    pSL->SaveDword(bFired);
    pSL->SaveDword(bReady2Fire);
    pSL->SaveDword(bRecharged);
    pSL->SaveDword(bLoad);
    pSL->SaveDword(bCanRecharge);
}

void AICannon::Load(CSaveLoad* pSL, AIAttributesHolder* _pAHolder, entid_t const eid)
{
    pAHolder  = _pAHolder;
    eidParent = eid;

    vPos              = pSL->LoadVector();
    vDir              = pSL->LoadVector();
    fTime2Action      = pSL->LoadFloat();
    fTotalTime2Action = pSL->LoadFloat();
    fSpeedV0          = pSL->LoadFloat();
    vEnemyPos         = pSL->LoadVector();
    bEmpty            = pSL->LoadDword() != 0;
    fDamaged          = pSL->LoadFloat();
    bFired            = pSL->LoadDword() != 0;
    bReady2Fire       = pSL->LoadDword() != 0;
    bRecharged        = pSL->LoadDword() != 0;
    bLoad             = pSL->LoadDword() != 0;
    bCanRecharge      = pSL->LoadDword() != 0;
}
