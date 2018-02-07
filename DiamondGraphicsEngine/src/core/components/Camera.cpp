#include "Precompiled.h"
#include "core/components/Camera.h"
#include "core/components/Transform.h"
#include "graphics/GraphicsEngine.h"
#include "core/TwImpl.h"

namespace TwCallBack
{
    void TW_CALL GetFov(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<float*>(value) = cam->GetFieldOfViewDegree();
    }
    void TW_CALL SetFov(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        float fov = *static_cast<const float*>(value);
        cam->SetFieldOfViewDegree(fov);
    }
    void TW_CALL GetWidth(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<float*>(value) = cam->GetWidth();
    }
    void TW_CALL SetWidth(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        float fov = *static_cast<const float*>(value);
        cam->SetWidth(fov);
    }
    void TW_CALL GetHeight(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<float*>(value) = cam->GetHeight();
    }
    void TW_CALL SetHeight(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        float fov = *static_cast<const float*>(value);
        cam->SetHeight(fov);
    }
    void TW_CALL GetNearPlane(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<float*>(value) = cam->GetNearPlaneDistance();
    }
    void TW_CALL SetNearPlane(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        float fov = *static_cast<const float*>(value);
        cam->SetNearPlaneDistance(fov);
    }
    void TW_CALL GetFarPlane(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<float*>(value) = cam->GetFarPlaneDistance();
    }
    void TW_CALL SetFarPlane(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        float fov = *static_cast<const float*>(value);
        cam->SetFarPlaneDistance(fov);
    }
    void TW_CALL GetViewVec(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<Math::Vector3*>(value) = cam->GetViewVector();
    }
    void TW_CALL SetViewVec(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        Math::Vector3 viewVec = *static_cast<const Math::Vector3*>(value);
        cam->SetViewVector(viewVec);
    }
    void TW_CALL GetCamEnable(void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        *static_cast<bool*>(value) = cam->IsEnabled();
    }
    void TW_CALL SetCamEnable(const void *value, void *clientData)
    {
        Component::Camera* cam = static_cast<Component::Camera*>(clientData);
        bool enabled = *static_cast<const bool*>(value);
        if (enabled)
        {
            cam->Enable();
        }
        else
        {
            cam->Disable();
        }
    }
}
namespace Component
{
    Camera::Camera(Transform& trans, bool defaultEnable, Graphics::GraphicsEngine* graphics)
        : ComponentBase(defaultEnable)
    , m_transform(trans), m_graphics(graphics)
    {
        if (defaultEnable)
        {
            Assert(graphics != nullptr, "graphics is NULL pointer, but camera is enabled.");
        }
    }

    Camera::~Camera()
    {
    }

    void Camera::Start()
    {
        if (m_isEnabled)
        {
            m_graphics->SetViewCamera(this, this);
        }
    }

    void Camera::Update(float)
    {
        //BUG redundant calculation
        OnCameraTransformChanged();
    }

    void Camera::Disable()
    {
        DEBUG_PRINT_DATA_FLOW
        if (m_isEnabled)
        {
            m_graphics->SetViewCamera(nullptr);
        }
        m_isEnabled = false;
    }

    void Camera::Reflect(TwBar* editor, std::string const& /*barName*/, std::string const& groupName, 
        Graphics::GraphicsEngine* /*graphics*/)
    {
        std::string defStr = "group=" + groupName;

        TwAddVarCB(editor, nullptr, TW_TYPE_BOOLCPP, TwCallBack::SetCamEnable, TwCallBack::GetCamEnable, this, (defStr + " label=Enabled").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_POINT(3, 0.01f, "X radians", "Y radians", "Z radians"), &m_position, (defStr + " label='Camera Local Position'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_POINT(3, 0.01f, "X radians", "Y radians", "Z radians"), &m_rotation, (defStr + " label='Camera Local Rotation'").c_str());
        TwAddVarCB(editor, nullptr, TW_TYPE_FLOAT, TwCallBack::SetFov, TwCallBack::GetFov, this, (defStr+" step=0.1 min=0 max=180  label='Field of View'").c_str());
        TwAddVarCB(editor, nullptr, TW_TYPE_FLOAT, TwCallBack::SetWidth, TwCallBack::GetWidth, this, (defStr+" step=0.1 min=0 label='Viewport Width'").c_str());
        TwAddVarCB(editor, nullptr, TW_TYPE_FLOAT, TwCallBack::SetHeight, TwCallBack::GetHeight, this, (defStr+" step=0.1 min=0 label='Viewport Height'").c_str());
        TwAddVarCB(editor, nullptr, TW_TYPE_FLOAT, TwCallBack::SetNearPlane, TwCallBack::GetNearPlane, this, (defStr+" step=0.1 min=0.01 label='Near Plane Distance'").c_str());
        TwAddVarCB(editor, nullptr, TW_TYPE_FLOAT, TwCallBack::SetFarPlane, TwCallBack::GetFarPlane, this, (defStr+" step=0.1 min=0.01 label='Far Plane Distance'").c_str());
        TwAddVarCB(editor, nullptr, TW_TYPE_DIR3F, TwCallBack::SetViewVec, TwCallBack::GetViewVec, this, (defStr+" label='View Vector'").c_str());
    }


    void Camera::OnEnable()
    {
        DEBUG_PRINT_DATA_FLOW
        m_graphics->SetViewCamera(this, this);
    }

    void Camera::CalcViewMatrix()
    {        
        Math::Matrix4 viewMat;
        Math::Matrix3 rotMatX;
        Math::Matrix3 rotMatY;
        Math::Matrix3 rotMatZ;
        rotMatX.Rotate(Math::Vector3::cXAxis, m_rotation.x + m_transform.GetRotationEuler().x);
        rotMatY.Rotate(Math::Vector3::cYAxis, m_rotation.y + m_transform.GetRotationEuler().y);
        rotMatZ.Rotate(Math::Vector3::cZAxis, m_rotation.z + m_transform.GetRotationEuler().z);

        Math::Matrix3 rotMat = rotMatZ* rotMatY*rotMatX;

        viewMat.BuildTransform(
            m_position + m_transform.GetPosition(),
            rotMat,
            Math::Vector3(1,1,1)
        );
        SetViewMatrix(viewMat.Inverted());
    }

    Math::Vector3 const& Camera::CalcViewVector()
    {
        Math::Matrix4 viewMat;
        Math::Matrix3 rotMatX;
        Math::Matrix3 rotMatY;
        Math::Matrix3 rotMatZ;
        rotMatX.Rotate(Math::Vector3::cXAxis, m_rotation.x + m_transform.GetRotationEuler().x);
        rotMatY.Rotate(Math::Vector3::cYAxis, m_rotation.y + m_transform.GetRotationEuler().y);
        rotMatZ.Rotate(Math::Vector3::cZAxis, m_rotation.z + m_transform.GetRotationEuler().z);

        Math::Matrix3 rotMat = rotMatZ* rotMatY*rotMatX;

        viewMat.BuildTransform(
            m_position + m_transform.GetPosition(),
            rotMat,
            Math::Vector3(1, 1, 1)
        );

        Math::Vector4 viewVec = Math::Vector4(DefaultViewDirection.x, DefaultViewDirection.y, DefaultViewDirection.z, 0);
        viewVec = Math::Transform(viewMat, viewVec);
        SetViewVector(Math::Vector3(viewVec.x, viewVec.y, viewVec.z));
        return GetViewVector();
    }

    Math::Vector3 const& Camera::CalcUpVector()
    {
        Math::Matrix4 viewMat;
        Math::Matrix3 rotMatX;
        Math::Matrix3 rotMatY;
        Math::Matrix3 rotMatZ;
        rotMatX.Rotate(Math::Vector3::cXAxis, m_rotation.x + m_transform.GetRotationEuler().x);
        rotMatY.Rotate(Math::Vector3::cYAxis, m_rotation.y + m_transform.GetRotationEuler().y);
        rotMatZ.Rotate(Math::Vector3::cZAxis, m_rotation.z + m_transform.GetRotationEuler().z);

        Math::Matrix3 rotMat = rotMatZ* rotMatY*rotMatX;

        viewMat.BuildTransform(
            m_position + m_transform.GetPosition(),
            rotMat,
            Math::Vector3(1, 1, 1)
        );

        Math::Vector4 upVec = Math::Vector4(DefaultUpDirection.x, DefaultUpDirection.y, DefaultUpDirection.z, 0);
        upVec = Math::Transform(viewMat, upVec);
        SetUpVector(Math::Vector3(upVec.x, upVec.y, upVec.z));
        return GetViewVector();
    }

    Math::Vector3 Camera::GetCameraWorldPosition() 
    {
        return m_position + m_transform.GetPosition();
    }

    Math::Vector3 Camera::GetCameraWorldRotationEuler()
    {
        return m_rotation + m_transform.GetRotationEuler();
    }

    void Camera::RotateCameraLocal(Math::Vector3 const& xyzRad)
    {
        m_rotation += xyzRad;
        OnCameraTransformChanged();
    }
}
