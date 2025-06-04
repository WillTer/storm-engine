#pragma once

#include <vector>

#include "../base_manager.h"

#define BIImagePrioritet_Group_Beg 15000

class BI_ManagerBase;

class BI_BaseGroup
{
public:
    BI_BaseGroup(BI_ManagerBase* pManager);
    virtual ~BI_BaseGroup();

    virtual void Init() {}

    virtual void Update() {}

    virtual int32_t Event(char const* pcEventName) = 0;

    BI_ManagerBase* Manager() const
    {
        return m_pManager;
    }

    std::vector<BI_ManagerNodeBase*> const& Nodes() const
    {
        return m_aNodes;
    }

protected:
    BI_ManagerBase*                  m_pManager;
    std::vector<BI_ManagerNodeBase*> m_aNodes;
};
