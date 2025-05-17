#pragma once

#include <libs/core/entity.h>
#include <libs/math/matrix.h>
#include <libs/math/triangle.h>
#include <libs/math/types3d.h>

using ADD_POLYGON_FUNC = bool (*)(const CVECTOR *v, int32_t nv);

class COLLISION_OBJECT : public Entity
{
  public:
    bool Init() override
    {
        return true;
    }

    ~COLLISION_OBJECT() override {};
    CMatrix mtx;

    virtual float Trace(const CVECTOR &src, const CVECTOR &dst) = 0;
    virtual bool Clip(const PLANE *planes, int32_t nplanes, const CVECTOR &center, float radius,
                      ADD_POLYGON_FUNC addpoly) = 0;

    virtual const char *GetCollideMaterialName() = 0;
    virtual bool GetCollideTriangle(TRIANGLE &triangle) = 0;
};
