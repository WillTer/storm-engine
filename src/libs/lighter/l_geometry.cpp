//============================================================================================
//    Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    LGeometry
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "l_geometry.h"

#include <libs/core/core.h>
#include <libs/core/entity.h>
#include <libs/filesystem/default_paths.h>
#include <libs/renderer/dx9render.h>


#ifdef _WIN32
#include <corecrt_io.h>
#else
#include <unistd.h>
#endif

// ============================================================================================
// Construction, destruction
// ============================================================================================

LGeometry::LGeometry() : min(), max(), useColor(false)
{
    numObjects    = 0;
    maxObjects    = 0;
    numVrt        = 0;
    maxVrt        = 0;
    numTrg        = 0;
    maxTrg        = 0;
    numVBuffers   = 0;
    maxVBuffers   = 0;
    shadows       = nullptr;
    drawbuf       = nullptr;
    radius        = 0.0f;
    modelsPath[0] = 0;
    lightPath[0]  = 0;
}

LGeometry::~LGeometry()
{
    delete shadows;
    delete drawbuf;
}

// Set path to models
void LGeometry::SetModelsPath(char const* mPath)
{
    strcpy_s(modelsPath, mPath);
}

// Set path for current weather
void LGeometry::SetLightPath(char const* lPath)
{
    strcpy_s(lightPath, lPath);
}

// Add object
void LGeometry::AddObject(char const* name, entid_t model)  // unused method?
{
    if (numObjects >= maxObjects) {
        maxObjects += 16;
        object.resize(maxObjects);
    }
    object[numObjects].nameReal = std::string(modelsPath) + name + ".gm";
    object[numObjects].path = fio->base_directory_path(BaseDirectory::Models) / modelsPath / (std::string(name) + "_" + lightPath + ".col");
    object[numObjects].model = model;
    object[numObjects].m     = static_cast<MODEL*>(core.GetEntityPointer(model));
    if (object[numObjects].m == nullptr) {
        core.Trace("Location lighter: can't get pointer to model %s", name);
    } else {
        ++numObjects;
    }
}

// Process data
bool LGeometry::Process(VDX9RENDER* rs, int32_t numLights)
{
    // Preparing data for lighting
    for (int32_t i = 0; i < numObjects; i++) {
        // Vertices ------------------------------------------------- -------------------------------
        // Index in the final file
        int32_t cindex = 0;
        // Check
        if (object[i].m != static_cast<MODEL*>(core.GetEntityPointer(object[i].model))) {
            core.Trace("Location lighter: lost model!!!");
            return false;
        }
        if (object[i].m->GetNode(1)) {
            core.Trace("Location lighter: incorrent model %s (nodes above 1)", object[i].nameReal.c_str());
            return false;
        }
        // Recalculate matrices
        object[i].m->Update();
        // get geometry
        auto* node = object[i].m->GetNode(0);
        auto* g    = node->geo;
        if (!g) {
            core.Trace("Location lighter: incorrent model %s (node not include geos)", object[i].nameReal.c_str());
            return false;
        }
        // Geometry information
        GEOS::INFO info;
        g->GetInfo(info);
        if (info.nvrtbuffs <= 0) {
            core.Trace("Location lighter: incorrent model %s (not vertex buffers), skip it", object[i].nameReal.c_str());
            object[i].lBufSize = 0;
            continue;
        }
        if (numVBuffers + info.nvrtbuffs > maxVBuffers) {
            maxVBuffers += info.nvrtbuffs + 16;
            vbuffer.resize(maxVBuffers);
        }
        for (int32_t vb = 0; vb < info.nvrtbuffs; vb++) {
            auto vbID = g->GetVertexBuffer(vb);
            if (vbID < 0) continue;
            vbuffer[numVBuffers].vbID    = vbID;
            vbuffer[numVBuffers++].start = numVrt;
            // get the vertices
            auto*                vbuf = rs->GetVertexBuffer(vbID);
            D3DVERTEXBUFFER_DESC desc;
            if (!vbuf || vbuf->GetDesc(&desc) != D3D_OK) {
                core.Trace("Location lighter: vertex buffer error, model %s, vbID %i", object[i].nameReal.c_str(), vbID);
                return false;
            }
            // Analyzing the type
            auto isEnabledType = true;
            isEnabledType &= ((desc.FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZ);
            isEnabledType &= ((desc.FVF & D3DFVF_NORMAL) != 0);
            isEnabledType &= ((desc.FVF & D3DFVF_DIFFUSE) != 0);
            isEnabledType &= ((desc.FVF & D3DFVF_PSIZE) == 0);
            if (!isEnabledType) {
                core.Trace("Location lighter: incorrect fvf of vertex buffer, model %s, vbID %i", object[i].nameReal.c_str(), vbID);
                return false;
            }
            // Vertex size
            int32_t stride = 6 * sizeof(float) + sizeof(uint32_t);
            stride += ((desc.FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT) * 2 * sizeof(float);
            if (desc.FVF & D3DFVF_SPECULAR) stride += sizeof(uint32_t);
            // Number of vertices
            auto num = desc.Size / stride;
            if (num <= 0) {
                core.Trace(
                    "Location lighter: incorrect number of verteces in vertex buffer, model %s, vbID %i", object[i].nameReal.c_str(), vbID);
                return false;
            }
            // reserve a place
            if (numVrt + num > maxVrt) {
                maxVrt = numVrt + num + 64;
                vrt.resize(maxVrt);
            }
            // Copy
            uint8_t* pnt = nullptr;
            if (vbuf->Lock(0, desc.Size, (void**)&pnt, 0) != D3D_OK) {
                core.Trace("Location lighter: vertex buffer no locked, model %s, vbID %i", object[i].nameReal.c_str(), vbID);
                return false;
            }
            for (int32_t v = 0; v < num; v++) {
                auto* pos     = (CVECTOR*)(pnt + v * stride);
                vrt[numVrt].p = *pos;
                auto* nrm     = (CVECTOR*)(pnt + v * stride + 3 * sizeof(float));
                vrt[numVrt].n = *nrm;
                auto color    = *(uint32_t*)(pnt + v * stride + 6 * sizeof(float));
                auto l        = ~vrt[numVrt].n;
                if (l > 0.0f) {
                    if (l != 1.0f) vrt[numVrt].n *= 1.0f / sqrtf(l);
                } else {
                    core.Trace(
                        "Location lighter: model %s, vbID %i, vrt: %i : normal have zero length", object[i].nameReal.c_str(), vbID, v);
                }
                vrt[numVrt].c      = 0.0f;
                vrt[numVrt].bc     = 0.0f;
                vrt[numVrt].mc     = 1.0f;
                vrt[numVrt].alpha  = 0xff000000;
                vrt[numVrt].flags  = Vertex::f_zero;
                vrt[numVrt].vbid   = vbID;
                vrt[numVrt].shadow = nullptr;
                vrt[numVrt].addr   = v * stride + 6 * sizeof(float);
                vrt[numVrt].obj    = i;
                vrt[numVrt].cindex = cindex++;
                if (useColor) {
                    vrt[numVrt].mc.x  = ((color >> 16) & 255) * (1.0f / 255.0f) * 2.0f;
                    vrt[numVrt].mc.y  = ((color >> 8) & 255) * (1.0f / 255.0f) * 2.0f;
                    vrt[numVrt].mc.z  = ((color >> 0) & 255) * (1.0f / 255.0f) * 2.0f;
                    vrt[numVrt].alpha = color & 0xff000000;
                }
                numVrt++;
            }
            vbuf->Unlock();
        }
        object[i].lBufSize = cindex;
        // Triangles ------------------------------------------------- -------------------------------
        auto  ibID = g->GetIndexBuffer();
        auto* idx  = static_cast<uint16_t*>(rs->LockIndexBuffer(ibID));
        if (!idx) {
            core.Trace("Location lighter: index buffer no locked, model %s", object[i].nameReal.c_str());
            return false;
        }
        GEOS::OBJECT obj;
        for (int32_t n = 0; n < info.nobjects; n++) {
            g->GetObj(n, obj);
            // looking for a vertex buffer
            int32_t vb;
            for (vb = 0; vb < numVBuffers; vb++)
                if (vbuffer[vb].vbID == static_cast<int32_t>(obj.vertex_buff)) break;
            if (vb >= numVBuffers) {
                core.Trace("Location lighter: vertex buffer %i not found, model %s", obj.vertex_buff, object[i].nameReal.c_str());
                return false;
            }
            vb = vbuffer[vb].start + obj.start_vertex;
            // Reading triangles
            auto* triangles = idx + obj.striangle * 3;
            for (int32_t t = 0; t < obj.ntriangles; t++) {
                // Relative indices
                int32_t i1 = triangles[t * 3 + 0];
                int32_t i2 = triangles[t * 3 + 1];
                int32_t i3 = triangles[t * 3 + 2];
                if (i1 >= obj.num_vertices || i2 >= obj.num_vertices || i3 >= obj.num_vertices) {
                    core.Trace(
                        "Location lighter: model %s have incorrect vertex index, (obj: %i, trg: %i)", object[i].nameReal.c_str(), n, t);
                    return false;
                }
                // Absolute indices
                i1 += vb;
                i2 += vb;
                i3 += vb;
                // Normal to triangle
                Assert(i1 >= 0 && i1 < numVrt);
                Assert(i2 >= 0 && i2 < numVrt);
                Assert(i3 >= 0 && i3 < numVrt);
                auto  nrm = ((vrt[i2].p - vrt[i1].p) ^ (vrt[i3].p - vrt[i1].p));
                float sq  = sqrtf(~nrm);
                // skip the empty triangle
                if (sq <= 0.0f) {
                    core.Trace("Location lighter: model %s have zero triangle, (obj: %i, trg: %i)", object[i].nameReal.c_str(), n, t);
                    continue;
                }
                // Add a triangle
                if (numTrg >= maxTrg) {
                    maxTrg += 256;
                    trg.resize(maxTrg);
                }
                trg[numTrg].n    = nrm * (1.0f / sq);
                trg[numTrg].sq   = sq;
                trg[numTrg].i[0] = i1;
                trg[numTrg].i[1] = i2;
                trg[numTrg].i[2] = i3;
                for (int32_t nv = 0; nv < 3; nv++) {
                    Vertex& vr    = vrt[trg[numTrg].i[nv]];
                    bool    isInv = (trg[numTrg].n | vr.n) < 0.0f;
                    if (vr.flags & Vertex::f_set) {
                        if (((vr.flags & Vertex::f_inv) != 0) != isInv) {
                            core.Trace("Location lighter: model %s have bug normals, (obj: %i, trg: %i)", object[i].nameReal.c_str(), n, t);
                            vr.flags |= Vertex::f_bug;
                        }
                    } else {
                        if (isInv) vr.flags |= Vertex::f_inv;
                    }
                }
                numTrg++;
            }
        }
    }
    if (numVrt <= 0) return false;
    shadows = new lighter::Shadow[numVrt * numLights];
    memset(shadows, 0, numVrt * numLights * sizeof(lighter::Shadow));
    min             = vrt[0].p;
    max             = vrt[0].p;
    int32_t lghtpnt = 0;
    for (int32_t i = 0; i < numVrt; i++) {
        Vertex& v = vrt[i];
        v.shadow  = shadows + lghtpnt;
        lghtpnt += numLights;
        if (v.p.x < min.x) min.x = v.p.x;
        if (v.p.y < min.y) min.y = v.p.y;
        if (v.p.z < min.z) min.z = v.p.z;
        if (v.p.x > max.x) max.x = v.p.x;
        if (v.p.y > max.y) max.y = v.p.y;
        if (v.p.z > max.z) max.z = v.p.z;
        if (v.flags & Vertex::f_inv) {
            if ((v.flags & Vertex::f_bug) == 0) {
                v.n.x = -v.n.x;
                v.n.y = -v.n.y;
                v.n.z = -v.n.z;
            }
        }
    }
    radius = sqrtf(~(max - min));
    return true;
}

// Draw normals
void LGeometry::DrawNormals(VDX9RENDER* rs)
{
    if (!drawbuf) drawbuf = new CVECTOR[1024];
    rs->SetRenderState(D3DRS_TEXTUREFACTOR, 0xff00ff00);
    int32_t p = 0;
    for (int32_t i = 0; i < numVrt; i++) {
        drawbuf[p + 0] = vrt[i].p;
        drawbuf[p + 1] = vrt[i].p + vrt[i].n;
        p += 2;
        if (p >= 1024) {
            rs->DrawPrimitiveUP(D3DPT_LINELIST, D3DFVF_XYZ, 512, drawbuf, sizeof(CVECTOR), "DbgDrawLines");
            p = 0;
        }
    }
    if (p > 1) { rs->DrawPrimitiveUP(D3DPT_LINELIST, D3DFVF_XYZ, p / 2, drawbuf, sizeof(CVECTOR), "DbgDrawLines"); }
}

// Update colors in buffers
void LGeometry::UpdateColors(VDX9RENDER* rs)
{
    int32_t  lockedVB = -1;
    uint8_t* pnt      = nullptr;
    for (int32_t i = 0; i < numVrt; i++) {
        if (vrt[i].vbid != lockedVB) {
            if (lockedVB >= 0) rs->UnLockVertexBuffer(lockedVB);
            lockedVB = -1;
            pnt      = static_cast<uint8_t*>(rs->LockVertexBuffer(vrt[i].vbid));
            if (!pnt) {
                core.Trace("Location lighter: no lock vertex buffer for update color");
                continue;
            }
            lockedVB = vrt[i].vbid;
        }
        CVECTOR c = vrt[i].c * vrt[i].mc * 255.0f;
        if (c.x < 0.0f) c.x = 0.0f;
        if (c.x > 255.0f) c.x = 255.0f;
        if (c.y < 0.0f) c.y = 0.0f;
        if (c.y > 255.0f) c.y = 255.0f;
        if (c.z < 0.0f) c.z = 0.0f;
        if (c.z > 255.0f) c.z = 255.0f;
        uint32_t& clr = *(uint32_t*)(pnt + vrt[i].addr);
        clr = (static_cast<uint32_t>(c.x) << 16) | (static_cast<uint32_t>(c.y) << 8) | (static_cast<uint32_t>(c.z) << 0) | vrt[i].alpha;
    }
    if (lockedVB >= 0) rs->UnLockVertexBuffer(lockedVB);
}

// Trace the ray
float LGeometry::Trace(const CVECTOR& src, const CVECTOR& dst)
{
    for (int32_t i = 0; i < numObjects; i++) {
        float const res = object[i].m->Trace(src, dst);
        if (res <= 1.0f) return res;
    }
    return 2.0f;
}

// Save lighting
bool LGeometry::Save()
{
    // Save the current path
    auto oldPath = fio->current_path();
    // Saving objects
    bool          result  = true;
    int32_t const bufSize = 16384;
    auto*         buf     = new uint32_t[bufSize];
    for (int32_t i = 0, pnt = 0; i < numObjects; i++) {
        if (object[i].lBufSize <= 0) continue;
        // Create a path
        fio->current_path(oldPath);
        if (!fio->create_directories(object[i].path.parent_path())) { continue; }

        auto stream = fio->open_file(object[i].path, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        if (!stream.is_open()) {
            result = false;
            continue;
        }
        int32_t sv = 0;
        for (int32_t j = 0, n = object[i].lBufSize; j < n; j++) {
            CVECTOR c = vrt[pnt].c * vrt[pnt].mc * 255.0f;
            pnt++;
            c.x = std::clamp(c.x, 0.0F, 255.0F);
            c.y = std::clamp(c.y, 0.0F, 255.0F);
            c.z = std::clamp(c.z, 0.0F, 255.0F);
            buf[sv++] =
                (static_cast<uint32_t>(c.x) << 16) | (static_cast<uint32_t>(c.y) << 8) | (static_cast<uint32_t>(c.z) << 0) | 0xff000000;
            if (sv >= bufSize) {
                stream.write(reinterpret_cast<char*>(buf), sv * sizeof(uint32_t));
                result = true;
                sv     = 0;
            }
        }
        if (sv > 0) { result &= true; }
    }
    fio->current_path(oldPath);
    delete[] buf;
    return result;
}
