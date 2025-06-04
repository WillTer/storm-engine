#pragma once

#include "../inode.h"

class INIFILE;
class XINTERFACE;

// video
class CXI_PICTURE: public CINODE
{
    friend XINTERFACE;

public:
    CXI_PICTURE();
    ~CXI_PICTURE() override;
    void Draw(bool bSelected, uint32_t Delta_Time) override;
    bool Init(
        INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, VDX9RENDER* rs, XYRECT& hostRect, XYPOINT& ScreenSize) override;
    void ReleaseAll() override;
    int  CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override {}

    void         ChangePosition(XYRECT& rNewPos) override;
    void         SaveParametersToIni() override;
    uint32_t     MessageProc(int32_t msgcode, MESSAGE& message) override;
    virtual void ChangeUV(FXYRECT& frNewUV);
    void         ChangeColor(uint32_t dwColor);
    void         SetPictureSize(int32_t& nWidth, int32_t& nHeight);

protected:
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;
    void SetNewPicture(bool video, char const* sNewTexName);
    void SetNewPictureFromDir(char const* dirName);
    void SetNewPictureByGroup(char const* groupName, char const* picName);
    void SetNewPictureByPointer(int32_t pTex);
    void ReleasePicture();

    char*            m_pcGroupName;
    int32_t          m_idTex;
    CVideoTexture*   m_pTex;
    XI_ONETEX_VERTEX m_v[4];

    bool     m_bMakeBlind;
    float    m_fCurBlindTime;
    bool     m_bBlindUp;
    float    m_fBlindUpSpeed;
    float    m_fBlindDownSpeed;
    uint32_t m_dwBlindMin;
    uint32_t m_dwBlindMax;
};
