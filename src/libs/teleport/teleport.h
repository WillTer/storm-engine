#pragma once

#include <libs/core/entity.h>

class TmpTeleport: public Entity
{
    struct TELEPORT_DESCR {
        int   num;
        char* name;
    }* m_descrArray;

public:
    TmpTeleport();
    ~TmpTeleport() override;
    bool     Init() override;
    void     Execute(uint32_t Delta_Time);
    void     Realize(uint32_t Delta_Time);
    uint64_t ProcessMessage(MESSAGE& message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize: Realize(delta); break;
        }
    }

protected:
    void ReleaseAll();
    void SetShowData(ATTRIBUTES* pA);
    void SortShowData();
    void XChange(TELEPORT_DESCR& d1, TELEPORT_DESCR& d2);

protected:
    int32_t m_leftPos;
    int32_t m_topPos;
    int32_t m_deltaPos;
    int32_t m_showStrQuantity;

    int32_t m_nStrQuantity;
    int32_t m_nCurStr;
    int32_t m_nCurShowPos;

    int32_t m_nShowType;
};

class FindFilesIntoDirectory: public Entity
{
public:
    FindFilesIntoDirectory() {}

    ~FindFilesIntoDirectory() override {}

    bool Init() override;

    void ProcessStage(Stage, uint32_t) override {}
};

class FindDialogNodes: public Entity
{
public:
    FindDialogNodes() {}

    ~FindDialogNodes() override {}

    bool Init() override;

    void ProcessStage(Stage, uint32_t) override {}
};
