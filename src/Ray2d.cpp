#include "Ray2d.h"

Ray2DCollision RayLineSegmentIntersection(Ray2D r, Vector2 point_a, Vector2 point_b)
{
    Vector2 line_vector = {point_b.x - point_a.x, point_b.y - point_a.y};

    float denominator = r.direction.x * line_vector.y - r.direction.y * line_vector.x;

    //If ray is nearly parallel
    if(abs(denominator) < 0.00001f)
    {
        return NO_RAY_COLLISION;
    }

    Vector2 Ray_to_a = {point_a.x - r.origin.x, point_a.y - r.origin.y};

    float t = (Ray_to_a.x * line_vector.y - Ray_to_a.y * line_vector.x) / denominator;
    float u = (Ray_to_a.x * r.direction.y - Ray_to_a.y * r.direction.x) / denominator;

    if(t < 0.0001f)return NO_RAY_COLLISION;

    //If is intersecting line segment
    if(t >= 0.0f && u >= 0.0f && u <= 1.0f)
    {
        Vector2 Intersection_point = {r.origin.x + t * r.direction.x, r.origin.y + t * r.direction.y};
        Vector2 Normal = Vector2Normalize({line_vector.y, -line_vector.x});
        return (Ray2DCollision){Intersection_point, Normal, t, true};
    }

    return NO_RAY_COLLISION;
}

void Ray2D::Reflect(Ray2DCollision c)
{
    direction = Vector2Reflect(direction, c.normal);
    origin = c.point;
}

void Ray2D::Refract(Ray2DCollision c, float refractive_index)
{
    float dot = direction.x*c.normal.x + direction.y*c.normal.y;
    float d = 1.0f - refractive_index*refractive_index*(1.0f - dot*dot);

    if (d >= 0.0f)
    {
        d = sqrtf(abs(d));
        direction.x = refractive_index*direction.x + (refractive_index*dot + d)*c.normal.x;
        direction.y = refractive_index*direction.y + (refractive_index*dot + d)*c.normal.y;
        origin = c.point;
    }
    else
    {
        Reflect(c);
    }
}
