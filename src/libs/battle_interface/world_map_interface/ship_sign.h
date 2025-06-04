#pragma once

#include <libs/battle_interface/bi_defines.h>

#include "../battle_sign.h"

#define MAX_SHIP_QUANTITY 8

class WMShipIcon: public BISignIcon
{
public:
    WMShipIcon(entid_t BIEntityID, VDX9RENDER* pRS);
    ~WMShipIcon() override;

    void ExecuteCommand(CommandType command) override;

protected:
    int32_t CalculateSignQuantity() override;
    void    UpdateChildrens() override;
};
