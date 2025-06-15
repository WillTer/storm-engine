#pragma once

#include <libs/core/vma.hpp>
#include <libs/renderer/dx9render.h>

#include "geometry.h"

//-------------------------------------------------------------------
// animated vertices
//-------------------------------------------------------------------
class GeometryService final: public VGEOMETRY
{
    VDX9RENDER* RenderService;

public:
    GeometryService();
    bool         Init() override;
    bool         LoadState(ENTITY_STATE* state) override;
    GEOS*        CreateGeometry(char const* file_name, char const* light_file_name, int32_t flags, char const* lmPath) override;
    void         DeleteGeometry(GEOS*) override;
    ANIMATION*   LoadAnimation(char const* anim) override;
    void         SetTechnique(char const* name) override;
    void         SetVBConvertFunc(VERTEX_TRANSFORM _transform_func) override;
    ANIMATION_VB GetAnimationVBDesc(int32_t avb) override;

    char const* GetTexturePath() override;
    void        SetTexturePath(char const*) override;

    void SetCausticMode(bool bSet = false) override;
};

class GEOM_SERVICE_R final: public GEOM_SERVICE
{
    static IDirect3DVertexDeclaration9* vertexDecl_;

    VDX9RENDER* RenderService;
    GEOS::ID    CurentIndexBuffer;
    GEOS::ID    CurentVertexBuffer;
    uint32_t    CurentVertexBufferSize;
    bool        bCaustic;

public:
    void SetRenderService(VDX9RENDER* render_service);

    std::ifstream OpenFile(char const* fname) override;
    int           FileSize(char const* fname) override;
    bool          ReadFile(std::ifstream& fileS, void* data, int32_t bytes) override;
    void          CloseFile(std::ifstream& fileS) override;
    void*         malloc(int32_t bytes) override;
    void          free(void* ptr) override;

    GEOS::ID CreateTexture(char const* fname) override;
    void     SetMaterial(const GEOS::MATERIAL& mt) override;
    void     ReleaseTexture(GEOS::ID tex) override;

    GEOS::ID CreateVertexBuffer(int32_t type, int32_t size) override;
    void*    LockVertexBuffer(GEOS::ID vb) override;
    void     UnlockVertexBuffer(GEOS::ID vb) override;
    void     ReleaseVertexBuffer(GEOS::ID vb) override;

    GEOS::ID CreateIndexBuffer(int32_t size) override;
    void*    LockIndexBuffer(GEOS::ID ib) override;
    void     UnlockIndexBuffer(GEOS::ID ib) override;
    void     ReleaseIndexBuffer(GEOS::ID ib) override;

    void SetIndexBuffer(GEOS::ID ibuff) override;
    void SetVertexBuffer(int32_t vsize, GEOS::ID vbuff) override;
    void DrawIndexedPrimitive(int32_t minv, int32_t numv, int32_t vrtsize, int32_t startidx, int32_t numtrg) override;

    GEOS::ID CreateLight(const GEOS::LIGHT) override;
    void     ActivateLight(GEOS::ID n) override;

    void SetCausticMode(bool bSet = false) override;
};

// API_SERVICE_START("geometry service")
//    DECLARE_MAIN_SERVICE(GEOMETRY)
// API_SERVICE_END(GEOMETRY)
