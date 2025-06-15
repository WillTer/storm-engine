#pragma once

#include "collide.h"

#pragma pack(push)
#pragma pack(1)

class LCOLL: public LOCAL_COLLIDE
{
    layer_index_t layerIndex_;
    PLANE         plane[6];
    CVECTOR       boxCenter;
    float         boxRadius;
    COLLIDE*      col;

public:
    LCOLL(layer_index_t idx);
    ~LCOLL() override;
    int32_t        SetBox(const CVECTOR& boxSize, CMatrix const& transform, bool testOnly = false) override;
    const CVECTOR* GetFace(int32_t& numVertices) override;
    float          Trace(const CVECTOR& src, const CVECTOR& dst) override;
};

class CollideService: public COLLIDE
{
public:
    CollideService()           = default;
    ~CollideService() override = default;
    LOCAL_COLLIDE* CreateLocalCollide(layer_index_t idx) override;
    float          Trace(entid_t entity, const CVECTOR& src, const CVECTOR& dst) override;
    float          Trace(
                 entity_container_cref entities, const CVECTOR& src, const CVECTOR& dst, entid_t const* exclude_list, int32_t exclude_num) override;
    bool Clip(
        entity_container_cref entities,
        const PLANE*          planes,
        int32_t               nplanes,
        const CVECTOR&        center,
        float                 radius,
        ADD_POLYGON_FUNC      addpoly,
        entid_t const*        exclude_list,
        int32_t               exclude_num) override;
    entid_t GetObjectID() override;
};

#pragma pack(pop)

// API_SERVICE_START("collide service")
//    DECLARE_MAIN_SERVICE(COLL)
// API_SERVICE_END(COLL)
