#pragma once

#include <vector>

#include <libs/island/island_base.h>
#include <libs/ship/ship_base.h>

// #define D3DTLVERTEX_FORMAT (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

using D3DTLVERTEX = struct {
    CVECTOR  vPos;
    float    fRHW;
    uint32_t dwDiffuse;
};

struct TOUCH_SHIP {
    entid_t      eID;                   // ship id
    CVECTOR      vContourTemp[2][128];  // temporary storage / must be deleted
    TOUCH_PARAMS TP[2];                 // touch params    for collision
    SHIP_BASE*   pShip;                 // ship pointer
    CVECTOR      vKickPoint;            // temp
    CVECTOR      vKickAngle;            // temp
    CVECTOR      vContour[128];         // initial contour // must be dynamic
    CVECTOR      vContourRect[4];       // rect contour
    int32_t      iNumVContour;          // num points in contour
};

class Touch: public Entity
{
protected:
    ISLAND_BASE* pIslandBase;

    bool bUseTouch;

    float fScale;
    float fCollisionDepth;

    uint32_t dwDeltaTime;
    int32_t  iDeltaTime;

    TOUCH_SHIP* pShips[256];
    int32_t     iNumShips;

    bool BuildContour(int32_t ship_idx);
    bool IsPointInContour(CVECTOR* vP, CVECTOR* vContour, int32_t numvcontour);

    bool IsSinked(int32_t iIndex);

    float   touch(int32_t idx, int32_t skip_idx, CVECTOR* vPos, CVECTOR* vAng, float fPower, float fSlide);
    bool    FakeTouch();
    bool    IsIntersectShipsRects(int32_t idx1, int32_t idx2);
    bool    IsIntersectShipsReal(int32_t idx1, int32_t cidx, CVECTOR* vPos, CVECTOR* vAng, CVECTOR* vRecoil, float* fPower, float* fSlide);
    void    GetLineABC(CVECTOR& v1, CVECTOR& v2, float& A, float& B, float& C);
    CVECTOR GetLineIntersectPoint(CVECTOR& v1, CVECTOR& v2, CVECTOR& o1, CVECTOR& o2);
    int32_t ProcessImpulse(int32_t iOurIdx, CVECTOR vPos, CVECTOR vDir, float fPowerApplied);
    int32_t GetTouchPoint(int32_t iIdx, const CVECTOR& vPos);

    // void    DrawLine(std::vector<RS_LINE2D>& aLines, float x1, float y1, float x2, float y2, uint32_t color);
    void    DrawShips();
    CVECTOR GetPoint(float x, float y, float xx, float yy, float xscale, float yscale, float fCos, float fSin, storm::Point ss);

    void SetDevices();

public:
    ~Touch() override;
    Touch();
    void     LoadServices();
    bool     Init() override;
    void     Realize(uint32_t Delta_Time);
    void     Execute(uint32_t Delta_Time);
    uint64_t ProcessMessage(MESSAGE& message) override;
    uint32_t AttributeChanged(ATTRIBUTES* pAttribute) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }
};
