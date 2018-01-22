#include "Precompiled.h"
#include "core/BoundingSphere.h"


BoundingSphere::BoundingSphere(Math::Vector3 const& c, float r)
    :radius(r), center(c)
{
}
