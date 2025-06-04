#pragma once

#include <string>

#include "image_defines.h"

class BIImageRender;

class BIString: public IBIString
{
public:
    BIString(BIString&&)      = delete;
    BIString(BIString const&) = delete;
    BIString(BIImageRender* pImgRender, VDX9RENDER* rs);
    ~BIString() override;

    int32_t GetPrioritet() const
    {
        return m_nPrioritet;
    }

    void SetPrioritet(int32_t nPrior)
    {
        m_nPrioritet = nPrior;
    }

    void Render();

    void RenderPrioritetRange(int32_t nBegPrior, int32_t nEndPrior)
    {
        if (m_nPrioritet >= nBegPrior && m_nPrioritet <= nEndPrior) Render();
    }

    void SetColor(uint32_t color) override
    {
        m_dwColor = color;
    }

    void SetScale(float fScale) override
    {
        m_fScale = fScale;
    }

    void SetFont(char const* pcFontName) override;
    void SetAlign(int32_t nHorzAlign, int32_t nVertAlign) override;
    void SetPosition(int32_t nLeft, int32_t nTop, int32_t nRight, int32_t nBottom) override;
    void SetString(char const* pcStr) override;

protected:
    void Release();
    void UpdateString();

    BIImageRender* m_pImgRender;
    VDX9RENDER*    m_pRS;
    FRECT          m_BasePos;
    int32_t        m_nPrioritet;
    std::string    m_sText;

    uint32_t m_dwColor;
    float    m_fScale;
    int32_t  m_nFontID;

    POINT m_pntOutPos {};

    bool m_bUpdateString;
};
