#pragma once

#include <libs/core/d_timer.h>
#include <libs/geometry/geometry.h>
#include <libs/sea/sea_base.h>
#include <libs/ship/ship_base.h>

#include "ai_helper.h"

class AISeaGoods: public Entity
{
private:
    struct item_t {
        char    sGoodName[48];
        int32_t iCharIndex, iQuantity;
        float   fTime;
        CVECTOR vPos, vNormal;
    };

    struct goods_t {
        std::string         sModel;
        std::vector<item_t> aItems;
        GEOS*               pGeo;

        goods_t() {};
    };

    std::vector<goods_t*>   aGoods;
    std::vector<SHIP_BASE*> aShips;

    SEA_BASE*   pSea;
    item_t      TmpItem;
    std::string sModelPath, sTmpModel;
    DTimer      dtCheckShips;
    bool        bDeleteGoodAnyway;
    float       fDistanceMultiply;

public:
    AISeaGoods();
    ~AISeaGoods() override;

    bool Init(std::shared_ptr<entt::registry> const& registry) override;
    void SetDevice();

    void Realize(uint32_t Delta_Time);
    void Execute(uint32_t Delta_Time);

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage) {
        case Stage::execute: Execute(delta); break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
              LostRender(delta); break;
            case Stage::restore_render:
              RestoreRender(delta); break;*/
        }
    }

    uint32_t AttributeChanged(ATTRIBUTES* pAttributeChanged) override;
};
