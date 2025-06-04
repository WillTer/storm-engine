#pragma once

#include <libs/battle_interface/bi_defines.h>

#include "base_node.h"

class IBIImage;

class BI_ImageNode: public BI_BaseNode
{
public:
    BI_ImageNode(BI_ManagerBase* pManager, char const* texture, const FRECT& uv, const RECT& pos, uint32_t color, int32_t nPrioritet);
    ~BI_ImageNode() override;

    void Update() override;

protected:
    IBIImage* m_pImage;
};
