#include "Precompiled.h"
#include "core/BoundingVolume.h"
#include "core/Ray.h"


BoundingSphere::BoundingSphere(Math::Vector3 const& c, float r)
    :radius(r), center(c)
{
}

bool BoundingSphere::CheckRayCollision(Ray const& ray, float* t) const
{
    using namespace Math;
    if (radius == 0.0f)
    {
        return false;
    }
    Vector3 direction = ray.GetRayDirection();
    Vector3 ray2sphere = center - ray.GetStartPosition();
    float radsq = radius * radius;
    float rs2center_sq = ray2sphere.LengthSq();
    if (rs2center_sq < radsq) //start from inside sphere
    {
        *t = 0;
        return true;
    }
    Vector3 temp_cross = (-ray2sphere).Cross(direction);
    float line2center_sq = temp_cross.Dot(temp_cross) / direction.Dot(direction);
    if (ray2sphere.Dot(direction) < 0)//direction is opposite to the sphere
    {
        return false;
    }
    if (line2center_sq > radsq)//outside
    {
        return false;
    }
    if (line2center_sq == radsq)//tanget - 1 solution
    {
        *t = sqrt(rs2center_sq - radsq);
        return true;
    }
    //intersects - 2 solutions
    *t = sqrt(rs2center_sq - line2center_sq) - sqrt(radsq - line2center_sq);
    return true;
}

BoundingAABB::BoundingAABB(Math::Vector3 const& _min, Math::Vector3 const& _max)
{
}

bool BoundingAABB::CheckRayCollision(Ray const& ray, float* t) const
{
    using namespace Math;
    bool pointAABB = false;

    Vector3 start = ray.GetStartPosition();
    Vector3 direction = ray.GetRayDirection();

    Vector3 cliped_pt = start;
    if (cliped_pt.x == Clamp(cliped_pt, aabbMin, aabbMax).x)
        if (cliped_pt.y == Clamp(cliped_pt, aabbMin, aabbMax).y)
            if (cliped_pt.z == Clamp(cliped_pt, aabbMin, aabbMax).z)
                pointAABB =  true;
    pointAABB =  false;


    if (pointAABB)
    {
        *t = 0;
        return true;
    }
    Vector3 pmin(aabbMin), pmax(aabbMax);
    for (int i = 0; i < 3; i++)//3 because of x,y,z
        (start[i] > aabbMax[i]) ? std::swap(pmin[i], pmax[i]) : 1;
    //-------------------------------------------------------------------
    // possible float division by 0, +-INF will be handled by std::max/min
    //-------------------------------------------------------------------
    float tmin = (pmin.x - start.x) / direction.x;
    tmin = std::max(tmin, (pmin.y - start.y) / direction.y);
    tmin = std::max(tmin, (pmin.z - start.z) / direction.z);
    float tmax = (pmax.x - start.x) / direction.x;
    tmax = std::min(tmax, (pmax.y - start.y) / direction.y);
    tmax = std::min(tmax, (pmax.z - start.z) / direction.z);
    if (tmin <= tmax)
    {
        *t = tmin;
        return true;
    }
    return false;
}
