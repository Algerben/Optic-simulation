#ifndef RAY2D_H
#define RAY2D_H

#include <raylib-cpp.hpp>

struct Ray2DCollision
{
    Vector2 point;
    Vector2 normal;
    float distance;
    bool hit;
};

struct Ray2D
{
    Vector2 origin;
    Vector2 direction;
    void Reflect(Ray2DCollision c);
    void Refract(Ray2DCollision c, float refractive_index);
};

#define NO_RAY_COLLISION (Ray2DCollision){{0,0}, {0,0}, 999999999.0f, false}

Ray2DCollision RayLineSegmentIntersection(Ray2D r, Vector2 point_a, Vector2 point_b);

#endif