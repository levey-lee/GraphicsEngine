#pragma once
#include "math/Vector3.h"


class Ray;
//todo finish bouding volume architecture
class BoundingVolume
{
public:
    virtual ~BoundingVolume() = 0 {}
    virtual bool CheckRayCollision(Ray const&,float*) const { return false; }

};



struct BoundingSphere : public BoundingVolume
{
    BoundingSphere(Math::Vector3 const& c = {0,0,0}, float r = 0.0f);
    bool CheckRayCollision(Ray const& ray, float* t) const override;


    float radius = 0.0f;
    Math::Vector3 center = { 0,0,0 };
};

struct BoundingAABB : public BoundingVolume
{
    BoundingAABB(Math::Vector3 const& _min = { 0,0,0 }, Math::Vector3 const& _max = { 0,0,0 });
    bool CheckRayCollision(Ray const& ray, float* t) const override;


    Math::Vector3 aabbMin = { 0,0,0 };
    Math::Vector3 aabbMax = { 0,0,0 };
};

