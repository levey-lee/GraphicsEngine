#include "Precompiled.h"
#include "graphics/CameraBase.h"

namespace Graphics
{
    /*******************************************************
     * @brief Default view camera based on graphics, not component.
     * This camera is not static or const, but it's never meant to be
     * used in actual project or game. Users should use component
     * based camera instead.
     *******************************************************/
    CameraBase CameraBase::DefaultCamera;
    const Math::Vector3 CameraBase::DefaultViewDirection = {0,0,-1};
    CameraBase::CameraBase()
    {
        *this = DefaultCamera;
    }

    Math::Vector3 const&CameraBase::CalcViewVector()
    {
		m_viewVec = {0,0,-1};
        return m_viewVec;
    }

    void CameraBase::SetViewVector(Math::Vector3 const& viewVec)
    {
        m_viewVec = viewVec;
    }

    void CameraBase::SetCameraUniforms(std::shared_ptr<ShaderProgram> shader)
	{//TODO(STUDENT): Set camera attributes to shader such as view vector,
     //camera position, fog, or anything global for all vertices.
	}

    void CameraBase::SetViewMatrix(Math::Matrix4 const& mat)
    {
        m_viewMatrix = mat;
    }

    void CameraBase::SetProjMatrix(Math::Matrix4 const& mat)
    {
        m_projMatrix = mat;
    }

    void CameraBase::SetViewProjMatrix(Math::Matrix4 const& mat)
    {
        m_viewProjMatrix = mat;
    }

    void CameraBase::CalcViewMatrix()
    {
        m_viewMatrix.SetIdentity();
    }

    void CameraBase::CalcProjMatrix()
    {
        m_projMatrix = Math::Matrix4::CreateProjection(m_fieldOfView, m_width, m_height, m_nearPlaneDist, m_farPlaneDist);
    }

    void CameraBase::CalcViewProjMatrix()
    {
        m_viewProjMatrix = m_projMatrix * m_viewMatrix;
    }

    void CameraBase::SetFieldOfViewRadians(float angleRadians)
    {
        m_fieldOfView = angleRadians;
        OnCameraViewportChanged();
    }

    void CameraBase::SetFieldOfViewDegree(float angleDegree)
    {
        m_fieldOfView = Math::DegToRad(angleDegree);
        OnCameraViewportChanged();
    }

    void CameraBase::SetDimension(float width, float height)
    {
        m_width = width;
        m_height = height;
        OnCameraViewportChanged();
    }

    void CameraBase::SetWidth(float w)
    {
        m_width = w;
        OnCameraViewportChanged();
    }

    void CameraBase::SetHeight(float h)
    {
        m_height = h;
        OnCameraViewportChanged();
    }

    void CameraBase::SetNearPlaneDistance(float zNear)
    {
        m_nearPlaneDist = zNear;
        OnCameraViewportChanged();
    }

    void CameraBase::SetFarPlaneDistance(float zFar)
    {
        m_farPlaneDist = zFar;
        OnCameraViewportChanged();
    }

    void CameraBase::OnCameraTransformChanged()
    {
        CalcViewMatrix();
        CalcViewProjMatrix();
        CalcViewVector();
    }

    void CameraBase::OnCameraViewportChanged()
    {
        CalcProjMatrix();
        CalcViewProjMatrix();
        CalcViewVector();
    }
}


