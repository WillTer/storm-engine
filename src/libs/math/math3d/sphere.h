//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Sphere
//============================================================================================

#pragma once

#pragma pack(push, 1)

#include "vector.h"
#include "vector4.h"

// Class for representing a sphere in 3D space
class Sphere
{
public:
    union {
        struct {
            // X Position
            float x;
            // Y position
            float y;
            // Z Position
            float z;
            // Radius
            float r;
        };

        // Vector4 representation
        Vector4 v4;
    };

    // -----------------------------------------------------------
    // Constructors
    // -----------------------------------------------------------
public:
    // Empty constructor
    Sphere();
    // Copy constructor
    Sphere(Sphere const& s);
    // -----------------------------------------------------------
    // Utilities
    // -----------------------------------------------------------
public:
    // Point in sphere
    bool Intersection(Vector const& p);
    // Check intersection of line and sphere
    bool Intersection(Vector const& src, Vector const& dst);
    // Check ray and sphere intersection
    bool Intersection(Vector const& orig, Vector const& normdir, float* res);
    // Check sphere and sphere intersection
    bool Intersection(Sphere const& sph);

    // Set sphere in a point with 0 radius
    void Reset(Vector const& p);
    // Include a point in the enclosing sphere
    void AddPoint(Vector const& p);

    // Check ray and sphere intersection
    static bool Intersection(Vector const& orig, Vector const& normdir, Vector const& pos, float r, float* res);
};

// -----------------------------------------------------------
// Constructors
// -----------------------------------------------------------

// Empty constructor
inline Sphere::Sphere()
{
    v4 = Vector4();
};

// Copy constructor
inline Sphere::Sphere(Sphere const& s)
{
    v4 = s.v4;
};

// ===========================================================
// Utilities
// ===========================================================

// Point in sphere
inline bool Sphere::Intersection(Vector const& p)
{
    return ~Vector(x - p.x, y - p.y, z - p.z) <= r * r;
}

// Check intersection of line and sphere
inline bool Sphere::Intersection(Vector const& src, Vector const& dst)
{
    Vector      dir = dst - src;
    float const len = dir.Normalize();
    if (len > 1e-10f) {
        float dist;
        if (!Intersection(src, dir, Vector(x, y, z), r, &dist)) return false;
        if (dist >= 0.0f) { return (dist <= len); }
        dir = -dir;
        if (!Intersection(dst, dir, Vector(x, y, z), r, &dist)) return false;
        if (dist >= 0.0f) { return (dist <= len); }
        return false;
    }
    return ~Vector(x - src.x, y - src.y, z - src.z) <= r * r;
}

// Check ray and sphere intersection
inline bool Sphere::Intersection(Vector const& orig, Vector const& normdir, float* res)
{
    return Intersection(orig, normdir, Vector(x, y, z), r, res);
}

// Check sphere and sphere intersection
inline bool Sphere::Intersection(Sphere const& sph)
{
    return (~Vector(x - sph.x, y - sph.y, z - sph.z) <= (r + sph.r) * (r + sph.r));
}

// Set sphere in a point with 0 radius
inline void Sphere::Reset(Vector const& p)
{
    x = p.x;
    y = p.y;
    z = p.z;
    r = 0.0f;
}

// Include a point in the enclosing sphere
inline void Sphere::AddPoint(Vector const& p)
{
    // Vector from point to center
    float const dx  = x - p.x;
    float const dy  = y - p.y;
    float const dz  = z - p.z;
    float       len = dx * dx + dy * dy + dz * dz;
    if (len <= r * r) return;
    len = sqrtf(len);
    // New radius
    r = (len + r) * 0.5f;
    // New position
    len = r / len;
    x   = p.x + dx * len;
    y   = p.y + dy * len;
    z   = p.z + dz * len;
}

// Check ray and sphere intersection
inline bool Sphere::Intersection(Vector const& orig, Vector const& normdir, Vector const& pos, float r, float* res)
{
    Vector const toCenter                   = pos - orig;
    float const  distToOrtoPlane            = normdir | toCenter;
    float const  distFromOrtoPlaneToSphere2 = r * r - (~toCenter - distToOrtoPlane * distToOrtoPlane);
    if (distFromOrtoPlaneToSphere2 < 0.0f) return false;
    if (res) { *res = distToOrtoPlane - sqrtf(distFromOrtoPlaneToSphere2); }
    return true;
}

#pragma pack(pop)
