#pragma once
#include "math/Vector3.h"

namespace Math {
    struct Matrix4;
}

/*******************************************************
 * @brief Ray class is used by the framework for raycasting.
 * When mouse left button is clicked, the camera will shoot
 * a ray towards its far plane's projected point. Then we do
 * ray casting to test if the ray hits any objects. If it hits,
 * we picks the nearest object as selected objcet.
 * This is how we select objects in a 3D scene.
 * @note For now it only support RaySphere collision test.
 * You could add more such as RayAABB, RayPlane etc.
 *******************************************************/
class Ray
{
public:
    Ray();
    Ray(Math::Vec3Param start, Math::Vec3Param dir);

    // Transforms this ray to another space by the given matrix 4
    Ray Transform(const Math::Matrix4& transform) const;

    // Returns the a point at the given t value.
    // t must be from [0, 1]
    Math::Vector3 GetPoint(float t) const;
    
    Math::Vector3 GetStartPosition() const { return m_start; }
    void SetStartPosition(Math::Vector3 const& pos) { m_start = pos; }

    Math::Vector3 GetRayDirection() const { return m_direction; }
    void SetRayDirection(Math::Vector3 const& dir) { m_direction = dir; }

    bool CheckCollisionSphere(const Math::Vector3& sphereCenter, float sphereRadius,
        float* t) const;
private:
    Math::Vector3 m_start;
    Math::Vector3 m_direction;
};
