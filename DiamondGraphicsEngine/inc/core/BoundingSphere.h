#pragma once
#include "math/Vector3.h"

struct BoundingSphere
{
    BoundingSphere(Math::Vector3 const& c = {0,0,0}, float r = 0.0f);
    float radius = 0.0f;
    Math::Vector3 center = { 0,0,0 };
};
