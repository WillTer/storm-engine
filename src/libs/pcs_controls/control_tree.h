#pragma once

#include <string>
#include <vector>

#include <libs/core/controls.h>

struct ControlTree {
public:
    struct ControlChild {
        ControlChild();
        ~ControlChild();

        void Process(float fDeltaTime, ControlTree* pControlTree);
        bool ControlInAction(ControlTree* pControlTree, char const* pcControlName, int32_t _nLayer);

        int32_t                   index;
        std::string               sControlName;
        std::vector<ControlChild> aChild;
        bool                      bActive;
        bool                      bWaitReleaseControl;
        float                     fTimeOut;
        float                     fCurTime;
        std::string               sOutControlName;
        int32_t                   nLayer;
    };

    struct OutControlInfo {
        std::string        sControlName;
        CONTROL_STATE_TYPE state;
    };

public:
    ControlTree();
    ~ControlTree();

    void Process();

    int32_t            AddControlChild(int32_t nParentIdx, char const* pcControlName, char const* pcOutControlName, float fTimeOut);
    CONTROL_STATE_TYPE CheckControlActivated(char const* pcControlName);

    bool AddOutControl(char const* pcOutControlName, bool isActive);
    void ControlInAction(char const* pcControlName, int32_t nLayer);
    bool ExcludeControlFromActive(char const* pcControlName);

protected:
    void          Init();
    void          Release();
    ControlChild* FindControlChild(int32_t idx);
    ControlChild* FindControlChild(int32_t idx, ControlChild* pParent);

protected:
    int32_t      m_nControlsNum;
    ControlChild m_RootControl;

    std::vector<OutControlInfo> m_aOutControlList;
};
