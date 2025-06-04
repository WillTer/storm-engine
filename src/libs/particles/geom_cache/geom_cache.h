#pragma once

#include <string>
#include <vector>

#include <libs/geometry/geometry.h>
#include <libs/geometry/geos.h>

class IGMXScene;

class GeomCache
{
    VGEOMETRY* pGS;

    struct CachedGeometry {
        GEOS*       pGeom;
        std::string FileName;
    };

    std::vector<CachedGeometry> Cache;

public:
    // Constructor / destructor
    GeomCache();
    ~GeomCache();

    // Put model into cache
    void CacheModel(char const* FileName);

    // Reset cache
    void ResetCache();

    // Get model from cache
    GEOS* GetModel(char const* FileName);

    // Check if such a model exists in the cache
    bool ValidatePointer(GEOS* pModel);
};
