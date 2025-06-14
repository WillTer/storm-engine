#pragma once

#include <libs/core/service.h>
#include <libs/core/vma.hpp>
#include <libs/geometry/geos.h>

class ANIMATION
{
public:
    virtual ~ANIMATION() {};
    virtual float SetTime(float time) = 0;
    virtual void  SetMatrix()         = 0;
};

using VERTEX_TRANSFORM = void* (*)(void* vb, int32_t startVrt, int32_t nVerts, int32_t totVerts);

class VGEOMETRY: public SERVICE
{
public:
    struct ANIMATION_VB {
        int32_t nvertices;
        int32_t fvf;
        int32_t stride;
        void*   buff;
    };

    virtual GEOS*      CreateGeometry(char const* file_name, char const* light_file_name, int32_t flags, char const* lmPath = nullptr) = 0;
    virtual void       DeleteGeometry(GEOS*)                                                                                           = 0;
    virtual ANIMATION* LoadAnimation(char const* anim)                                                                                 = 0;
    virtual void       SetTechnique(char const* name)                                                                                  = 0;
    virtual void       SetVBConvertFunc(VERTEX_TRANSFORM _transform_func)                                                              = 0;
    virtual ANIMATION_VB GetAnimationVBDesc(int32_t avb)                                                                               = 0;

    virtual char const* GetTexturePath()            = 0;
    virtual void        SetTexturePath(char const*) = 0;

    virtual void SetCausticMode(bool bSet = false) = 0;
};

using GeometryPtr = std::shared_ptr<VGEOMETRY>;