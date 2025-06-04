//============================================================================================
// Spirenkov Maxim, 2003
//============================================================================================
// Plane
//============================================================================================

#pragma once

#pragma pack(push, 1)

#include "vector.h"

// Class for representing a plane in 3D space
class Plane
{
public:
    // Normal
    Vector N;
    // Distance from center
    float D;

    // -----------------------------------------------------------
    // Constructors
    // -----------------------------------------------------------
public:
    // Empty constructor
    Plane();
    // Set direction
    Plane(float Nx, float Ny, float Nz);
    // Set direction
    Plane(Vector const& normal);
    // Create plane
    Plane(Vector const& normal, Vector const& point);
    // Copy constructor
    Plane(Plane const& plane);

    // -----------------------------------------------------------
    // Transformation
    // -----------------------------------------------------------
public:
    // Normalize
    Plane& Normalize();
    // Move plane to specified point
    Plane& Move(Vector const& point);

    // -----------------------------------------------------------
    // Utilities
    // -----------------------------------------------------------
public:
    // Find distance to plane (*)
    float Dist(Vector const& point) const;
    // Check for intersection of line and plane
    bool Intersection(Vector const& src, Vector const& dst) const;
    // Find the intersection point of a line and a plane
    bool Intersection(Vector const& src, Vector const& dst, Vector& res) const;
    // Check for line and plane intersection
    bool IntersectionLine(Vector const& src, Vector const& dst, float& k) const;
};

// ===========================================================
// Constructors
// ===========================================================

// Empty constructor
inline Plane::Plane() {}

// Set direction
inline Plane::Plane(float Nx, float Ny, float Nz)
{
    N.x = Nx;
    N.y = Ny;
    N.z = Nz;
    D   = 0.0f;
}

// Set direction
inline Plane::Plane(Vector const& normal)
{
    N = normal;
}

// Create plane
inline Plane::Plane(Vector const& normal, Vector const& point)
{
    N = normal;
    D = normal | point;
}

// Copy constructor
inline Plane::Plane(Plane const& plane)
{
    N = plane.N;
    D = plane.D;
}

// ===========================================================
// Operators
// ===========================================================

/*!\relates Plane
Distance from point to plane
*/
inline float operator*(Vector const& point, Plane const& plane)
{
    return (plane.N | point) - plane.D;
}

/*!\relates Plane
Distance from point to plane
*/
inline float operator*(Plane const& plane, Vector const& point)
{
    return (plane.N | point) - plane.D;
}

// ===========================================================
// Transformation
// ===========================================================

// Normalize
inline Plane& Plane::Normalize()
{
    auto const d = N.Normalize();
    if (d != 0.0f)
        D /= d;
    else
        D = 0.0f;
    return *this;
}

// Move plane to specified point
inline Plane& Plane::Move(Vector const& point)
{
    D = (N | point);
    return *this;
}

// ===========================================================
// Utilities
// ===========================================================

// Find distance to plane (*)
inline float Plane::Dist(Vector const& point) const
{
    return *this * point;
}

// Check for intersection of line and plane
inline bool Plane::Intersection(Vector const& src, Vector const& dst) const
{
    auto const dsrc = *this * src;
    auto const ddst = *this * dst;
    return (dsrc * ddst <= 0.0f);
}

// Find the intersection point of a line and a plane
inline bool Plane::Intersection(Vector const& src, Vector const& dst, Vector& res) const
{
    auto const dsrc = *this * src;
    auto       ddst = *this * dst;
    if (dsrc * ddst > 0.0f) return false;
    ddst = dsrc - ddst;
    res  = src;
    if (ddst != 0.0f) res += (dst - src) * dsrc / ddst;
    return true;
}

// Check for line and plane intersection
inline bool Plane::IntersectionLine(Vector const& src, Vector const& dst, float& k) const
{
    auto const dsrc = *this * src;
    auto       ddst = *this * dst;
    ddst            = dsrc - ddst;
    if (fabsf(ddst) <= 1e-30f) return false;
    k = dsrc / ddst;
    return true;
}

#pragma pack(pop)
