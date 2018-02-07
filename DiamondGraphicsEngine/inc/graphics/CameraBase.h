#pragma once
#include "math/Matrix4.h"

namespace Graphics
{
    class ShaderProgram;

    /*******************************************************
     * @brief Camera base class, can be used directly.
     * Usually we use a camera component instead of a camera
     * that comes from the graphics engine. 
     *******************************************************/
    class CameraBase
    {
    public:
        CameraBase();
        virtual ~CameraBase() = default;

        virtual Math::Vector3 const& CalcViewVector();
        virtual Math::Vector3 const& GetViewVector() const { return m_viewVec; }
        virtual void SetViewVector(Math::Vector3 const& viewVec);

        virtual Math::Vector3 const& GetUpVector() const { return m_upVec; }
        virtual Math::Vector3 const& CalcUpVector();
        virtual void SetUpVector(Math::Vector3 const& upVec);

        virtual Math::Matrix4 const& GetViewMatrix() { return m_viewMatrix; }
        virtual Math::Matrix4 const& GetProjMatrix() { return m_projMatrix; }
        virtual Math::Matrix4 const& GetViewProjMatrix() { return m_viewProjMatrix; }
        
		virtual void SetCameraUniforms(std::shared_ptr<ShaderProgram> shader);

        virtual void SetViewMatrix(Math::Matrix4 const& mat);
        virtual void SetProjMatrix(Math::Matrix4 const& mat);
        virtual void SetViewProjMatrix(Math::Matrix4 const& mat);

        virtual void CalcViewMatrix();
        virtual void CalcProjMatrix();
        virtual void CalcViewProjMatrix();

        virtual float GetFieldOfViewRadians() { return m_fieldOfView; }
        virtual void SetFieldOfViewRadians(float angleRadians);
        virtual float GetFieldOfViewDegree() { return Math::RadToDeg(m_fieldOfView); }
        virtual void SetFieldOfViewDegree(float angleDegree);

        virtual Math::Vector2 GetDimension() const { return { m_width, m_height }; }
        virtual void SetDimension(float width, float height);

        virtual float GetWidth() { return m_width; }
        virtual void SetWidth(float w);

        virtual float GetHeight() { return m_height; }
        virtual void SetHeight(float h);

        virtual float GetNearPlaneDistance() { return m_nearPlaneDist; }
        virtual void SetNearPlaneDistance(float zNear);

        virtual float GetFarPlaneDistance() { return m_farPlaneDist; }
        virtual void SetFarPlaneDistance(float zFar);

        virtual Math::Vector3 GetCameraLocalPosition() { return {}; }
        virtual Math::Vector3 GetCameraWorldPosition() { return {}; }
        virtual Math::Vector3 GetCameraLocalRotationEuler() { return {}; }
        virtual Math::Vector3 GetCameraWorldRotationEuler() { return {}; }
        virtual void RotateCameraLocal(Math::Vector3 const& xyzRad) {}

        static CameraBase DefaultCamera;
        static const Math::Vector3 DefaultViewDirection;
        static const Math::Vector3 DefaultUpDirection;

    protected:

        virtual void OnCameraTransformChanged();
        virtual void OnCameraViewportChanged();
        
    private:
        float m_fieldOfView = Math::c_Pi/3.0f;
        float m_width = 160.0f;
        float m_height = 90.0f;
        float m_nearPlaneDist = 1.0f;
        float m_farPlaneDist = 1000.0f;

        Math::Vector3 m_viewVec = DefaultViewDirection;
        Math::Vector3 m_upVec = DefaultUpDirection;
        Math::Matrix4 m_viewMatrix;
        Math::Matrix4 m_projMatrix;
        Math::Matrix4 m_viewProjMatrix;

    };
}
