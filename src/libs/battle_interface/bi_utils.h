#pragma once

#include <string>
#include <vector>

#include "bi_defines.h"

class BITextInfo final
{
public:
    BITextInfo(BITextInfo&& text_info) noexcept;

    BITextInfo(BITextInfo const& text_info);

    BITextInfo();
    ~BITextInfo();
    void Release();
    void Init(/*VDX9RENDER*/ void* rs, ATTRIBUTES* pA);
    void Print();
    void Print(std::string outputText);

    // FIXME: Renderer Next
    // VDX9RENDER* pRS;
    std::string  sText;
    storm::Point pos;
    float        fScale;
    int32_t      nFont;
    uint32_t     dwColor;
    bool         bShadow;

    ATTRIBUTES* pARefresh;
};

class BILinesInfo
{
public:
    BILinesInfo();
    ~BILinesInfo();
    void Release();
    void Init(/*VDX9RENDER*/ void* rs, ATTRIBUTES* pA);
    void Draw();

    // FIXME: Renderer Next
    // VDX9RENDER*            pRS;
    // std::vector<RS_LINE2D> lines;
};

class IBIImage;
class BIImageRender;

class BIImagesInfo
{
public:
    BIImagesInfo();
    ~BIImagesInfo();
    void Release();
    void Init(/*VDX9RENDER*/ void* rs, ATTRIBUTES* pA);
    void Draw() const;

    // FIXME: Renderer Next
    // VDX9RENDER*            pRS;
    BIImageRender*         pImgRender;
    std::vector<IBIImage*> images;
};

class BIBorderInfo
{
public:
    BIBorderInfo();
    ~BIBorderInfo();
    void Release();
    void Init(/*VDX9RENDER*/ void* rs, ATTRIBUTES* pA);
    void Draw();

    // FIXME: Renderer Next
    // VDX9RENDER* pRS;
    int32_t      nVBuf;
    int32_t      nTexID;
    storm::FRect ext_pos;
    storm::FRect int_pos1;
    storm::FRect int_pos2;
    uint32_t     dwColor1;
    uint32_t     dwColor2;
    float        fCur;
    float        fSpeed;
    bool         bUp;
    bool         bUsed;
};

class BIUtils
{
    //---------------------------------------
public:  // functions
    static int32_t     GetIntFromAttr(ATTRIBUTES* pA, char const* name, int32_t defVal);
    static float       GetFloatFromAttr(ATTRIBUTES* pA, char const* name, float defVal);
    static bool        ReadStringFromAttr(ATTRIBUTES* pA, char const* name, char* buf, int32_t bufSize, char const* defVal);
    static char const* GetStringFromAttr(ATTRIBUTES* pA, char const* name, char const* defVal);
    static int32_t     GetTextureFromAttr(/*VDX9RENDER*/ void* rs, ATTRIBUTES* pA, char const* sAttrName);
    static bool        ReadRectFromAttr(ATTRIBUTES* pA, char const* name, storm::FRect& rOut, storm::FRect& rDefault);
    static bool        ReadRectFromAttr(ATTRIBUTES* pA, char const* name, storm::Rect& rOut, storm::Rect& rDefault);
    static bool        ReadPosFromAttr(ATTRIBUTES* pA, char const* name, float& fX, float& fY, float fXDef, float fYDef);
    static bool        ReadPosFromAttr(ATTRIBUTES* pA, char const* name, int32_t& nX, int32_t& nY, int32_t nXDef, int32_t nYDef);
    static int32_t     GetAlignmentFromAttr(ATTRIBUTES* pA, char const* name, int32_t nDefAlign);
    static int32_t     GetFontIDFromAttr(ATTRIBUTES* pA, char const* name, /*VDX9RENDER*/ void* rs, char const* pcDefFontName);
    static bool        ReadVectorFormAttr(ATTRIBUTES* pA, char const* name, CVECTOR& vOut, const CVECTOR& vDef);

    static bool ComparePoint(storm::Point& p1, storm::Point& p2);

    static ATTRIBUTES* GetAttributesFromPath(ATTRIBUTES* pA, ...);

    static uint32_t GetIntervalColor(uint32_t minV, uint32_t maxV, float fpar);
    static bool     GetIntervalRect(float fk, storm::FRect const& r1, storm::FRect const& r2, storm::FRect& rOut);

    static int32_t GetMaxFromFourLong(int32_t n1, int32_t n2, int32_t n3, int32_t n4);

    static float GetFromStr_Float(char const*& pcStr, float fDefault);

    static void FillTextInfoArray(/*VDX9RENDER*/ void* pRS, ATTRIBUTES* pA, std::vector<BITextInfo>& tia);
    static void PrintTextInfoArray(std::vector<BITextInfo>& tia);
    //---------------------------------------
    //---------------------------------------
public:  // data
    static entid_t  idBattleInterface;
    static uint32_t g_dwBlinkColor;
    //---------------------------------------
};
