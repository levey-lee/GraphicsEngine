#include "Precompiled.h"
#include "core/Ray.h"
#include "math/Matrix4.h"

Ray::Ray()
{
    m_start = m_direction = Math::Vector3::cZero;
}

Ray::Ray(Math::Vec3Param start, Math::Vec3Param dir)
{
    m_start = start;
    m_direction = dir;
}

Ray Ray::Transform(const Math::Matrix4& transform) const
{
    Ray transformedRay;
    transformedRay.m_start = Math::TransformPoint(transform, m_start);
    transformedRay.m_direction = Math::TransformNormal(transform, m_direction);
    return transformedRay;
}

Math::Vector3 Ray::GetPoint(float t) const
{
    return m_start + m_direction * t;
}

bool Ray::CheckCollisionSphere(const Math::Vector3& sphereCenter, float sphereRadius, float* t) const
{
    using namespace Math;
    if (sphereRadius == 0.0f)
    {
        return false;
    }
    Vector3 ray2sphere = sphereCenter - m_start;
    float radsq = sphereRadius * sphereRadius;
    float rs2center_sq = ray2sphere.LengthSq();
    if (rs2center_sq < radsq) //start from inside sphere
    {
        *t = 0;
        return true;
    }
    Vector3 temp_cross = (-ray2sphere).Cross(m_direction);
    float line2center_sq = temp_cross.Dot(temp_cross) / m_direction.Dot(m_direction);
    if (ray2sphere.Dot(m_direction) < 0)//direction is opposite to the sphere
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
