#pragma once

#include <vector>

#include "image_defines.h"

class BIImageMaterial;

class BIImage: public IBIImage
{
public:
    BIImage(/*VDX9RENDER*/ void* rs, BIImageMaterial* pMaterial);
    ~BIImage() override;

    size_t GetVertexQuantity() const
    {
        return m_nVertexQuantity;
    }

    size_t GetTriangleQuantity() const
    {
        return m_nTriangleQuantity;
    }

    void FillBuffers(BI_IMAGE_VERTEX* pV, uint16_t* pT, size_t& nV, size_t& nT);

    void SetColor(uint32_t color) override;
    void SetPosition(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom) override;
    void Set3DPosition(const CVECTOR& vPos, float fWidth, float fHeight) override;
    void SetUV(storm::FRect const& uv) override;
    void SetType(BIImageType type);

    void           CutSide(float fleft, float fright, float ftop, float fbottom) override;
    void           CutClock(float fBegin, float fEnd, float fFactor) override;
    storm::FPoint& GetClockPoint(float fAng, storm::FPoint& fp);
    float          GetNextClockCorner(float fAng);
    float          GetPrevClockCorner(float fAng);

    int32_t GetPrioritet() const
    {
        return m_nPrioritet;
    }

    void SetPrioritet(int32_t nPrior)
    {
        m_nPrioritet = nPrior;
    }

protected:
    float CalculateMidPos(float fMin, float fMax, float fK)
    {
        return fMin + fK * (fMax - fMin);
    }

    void Release() const;

    // VDX9RENDER*      m_pRS;
    BIImageMaterial* m_pMaterial;

    size_t m_nVertexQuantity;
    size_t m_nTriangleQuantity;

    storm::FRect m_BasePos;
    storm::FRect m_BaseUV;
    uint32_t     m_dwColor;

    std::vector<storm::FPoint> m_aRelPos;
    BIImageType                m_eType;

    int32_t m_nPrioritet;
};
