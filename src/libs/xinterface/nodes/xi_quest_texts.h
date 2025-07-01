#pragma once

#include "../inode.h"

class CXI_QUESTTEXTS: public CINODE
{
public:
    CXI_QUESTTEXTS(CXI_QUESTTEXTS&&)      = delete;
    CXI_QUESTTEXTS(const CXI_QUESTTEXTS&) = delete;
    CXI_QUESTTEXTS();
    ~CXI_QUESTTEXTS() override;

    void Draw(storm::GPURenderPass const& render_pass, bool bSelected, uint32_t Delta_Time) override;
    bool
    Init(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2, /*VDX9RENDER*/ void* rs, XYRECT& hostRect, XYPOINT& ScreenSize)
        override;
    void ReleaseAll() override;
    int  CommandExecute(int wActCode) override;
    bool IsClick(int buttonID, int32_t xPos, int32_t yPos) override;

    void MouseThis(float fX, float fY) override {}

    void     ChangePosition(XYRECT& rNewPos) override;
    void     SaveParametersToIni() override;
    uint32_t MessageProc(int32_t msgcode, MESSAGE& message) override;

    void StartQuestShow(ATTRIBUTES* pA, int qn);

    float GetLineStep() const;
    void  ScrollerChanged(float fPos);

protected:
    bool GetLineNext(int fontNum, char*& pInStr, char* buf, int bufSize) const;
    void LoadIni(INIFILE* ini1, char const* name1, INIFILE* ini2, char const* name2) override;
    void ReleaseStringes();
    void GetStringListForQuestRecord(std::vector<std::string>& asStringList, char const* pcRecText, char const* pcUserData) const;

    int32_t  m_idFont;
    uint32_t m_dwNonCompleteColor;
    uint32_t m_dwCompleteColor;

    int m_allStrings;
    int m_vertOffset;

    struct STRING_DESCRIBER {
        int               strNum;
        char*             lineStr;
        bool              complete;
        STRING_DESCRIBER* next;
        STRING_DESCRIBER* prev;
        STRING_DESCRIBER* Add(char const* ls, bool complete);
        STRING_DESCRIBER(char const* ls);
    };

    STRING_DESCRIBER* m_listRoot;
    STRING_DESCRIBER* m_listCur;
    int               m_nAllTextStrings;
};
