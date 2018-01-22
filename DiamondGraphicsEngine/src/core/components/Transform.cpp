#include "Precompiled.h"
#include "core/components/Transform.h"
#include "core/Scene.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/CameraBase.h"
#include "core/components/Renderer.h"
#include "core/TwImpl.h"
#include "graphics/ShaderProgram.h"


namespace TwCallBack
{
    void TW_CALL GetPosition(void *value, void *clientData)
    {
        Component::Transform* transform = static_cast<Component::Transform*>(clientData);
        *static_cast<Math::Vector3*>(value) = transform->GetPosition();        
    }
    void TW_CALL SetPosition(const void *value, void *clientData)
    {
        Component::Transform* transform = static_cast<Component::Transform*>(clientData);
        Math::Vector3 pos = *static_cast<const Math::Vector3*>(value);
        transform->SetPosition(pos);
    }
    void TW_CALL GetRotation(void *value, void *clientData)
    {
        Component::Transform* transform = static_cast<Component::Transform*>(clientData);
        Math::Vector3 rotRad = transform->GetRotationEular();
        Math::Vector3 rotDeg = { Math::RadToDeg(rotRad.x),Math::RadToDeg(rotRad.y) ,Math::RadToDeg(rotRad.z) };
        *static_cast<Math::Vector3*>(value) = rotDeg;
    }
    void TW_CALL SetRotation(const void *value, void *clientData)
    {
        Component::Transform* transform = static_cast<Component::Transform*>(clientData);
        Math::Vector3 rotDeg = *static_cast<const Math::Vector3*>(value);
        Math::Vector3 rotRad = { Math::DegToRad(rotDeg.x),Math::DegToRad(rotDeg.y) ,Math::DegToRad(rotDeg.z) };
        transform->SetRotation(rotRad);
    }
    void TW_CALL GetScale(void *value, void *clientData)
    {
        Component::Transform* transform = static_cast<Component::Transform*>(clientData);
        *static_cast<Math::Vector3*>(value) = transform->GetScale();
    }
    void TW_CALL SetScale(const void *value, void *clientData)
    {
        Component::Transform* transform = static_cast<Component::Transform*>(clientData);
        Math::Vector3 scale = *static_cast<const Math::Vector3*>(value);
        transform->SetScale(scale);
    }

}


void Component::Transform::SetShaderParams(std::shared_ptr<Graphics::ShaderProgram> shader,
    Graphics::GraphicsEngine* graphics)
{
    std::shared_ptr<Graphics::ShaderManager> shaderManager = graphics->GetShaderManager();
    //TODO(Assignment 1): Set transform to shader
    ////////////////////////////////////////////////////////////////////////////
    //  set transform
#if SAMPLE_IMPLEMENTATION
    Math::Matrix4 const& modelMatrix = GetWorldTransform();
    Math::Matrix4 mvp = graphics->GetViewCamera()->GetViewProjMatrix() * modelMatrix;
    shader->SetUniform("ModelMatrix", modelMatrix);
    shader->SetUniform("ModelViewProjectionMatrix", mvp);
#else
    Math::Matrix4 sampleModelMatrix = Math::Matrix4::c_Identity;
    Math::Matrix4 sampleMvpMatrix = Math::Matrix4::c_Identity;
    sampleModelMatrix[2][3] = -2;//Hardcode position.z so you can see the model;
    sampleMvpMatrix.array[0] = 2.41421342f;
    sampleMvpMatrix.array[5] = 4.29193497f;
    sampleMvpMatrix.array[10] = -1.00200200f;
    sampleMvpMatrix.array[11] = 1.80380380f;
    sampleMvpMatrix.array[14] = -1.00000000f;
    sampleMvpMatrix.array[15] = 2.00000000f;
    shader->SetUniform("ModelMatrix", sampleModelMatrix);
    shader->SetUniform("ModelViewProjectionMatrix", sampleMvpMatrix);
#endif
}

Component::Transform& Component::Transform::Translate(Math::Vector3 const& trans)
{
    m_position += trans;
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::Scale(Math::Vector3 const& scale)
{
    m_scale += scale;
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::Scale(float uniformScale)
{
    m_scale += {uniformScale, uniformScale, uniformScale};
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::Rotate(Math::Vector3 const& angleRadianXYZ)
{
    m_rotationEuler += angleRadianXYZ;
    if (m_rotationEuler.x > Math::c_TwoPi)
    {
        m_rotationEuler.x -= Math::c_TwoPi;
    }
    else if (m_rotationEuler.x < -Math::c_TwoPi)
    {
        m_rotationEuler.x += Math::c_TwoPi;
    }
    if (m_rotationEuler.y > Math::c_TwoPi)
    {
        m_rotationEuler.y -= Math::c_TwoPi;
    }
    else if (m_rotationEuler.x < -Math::c_TwoPi)
    {
        m_rotationEuler.y += Math::c_TwoPi;
    }
    if (m_rotationEuler.z > Math::c_TwoPi)
    {
        m_rotationEuler.z -= Math::c_TwoPi;
    }
    else if (m_rotationEuler.z < -Math::c_TwoPi)
    {
        m_rotationEuler.z += Math::c_TwoPi;
    }
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::Orbit(Math::Vector3 const& axis, float angleRadian)
{
    Math::Matrix3 rotMat;
    rotMat.SetIdentity();
    rotMat.Rotate(axis, angleRadian);
    m_position = Math::Transform(rotMat, m_position);
    OnTransformChanged();
    return *this;
}

Math::Matrix4 const& Component::Transform::GetLocalTransform() const
{
    return m_localTransform;
}

Math::Matrix4 const& Component::Transform::GetWorldTransform() const
{
    return m_worldTransform;
}

Component::Transform& Component::Transform::SetPosition(Math::Vector3 const& pos)
{
    m_position = pos;
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::SetRotation(Math::Vector3 const& rotEuler)
{
    m_rotationEuler = rotEuler;
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::SetScale(Math::Vector3 const& scale)
{
    m_scale = scale;
    OnTransformChanged();
    return *this;
}

Component::Transform& Component::Transform::SetScale(float uniformScale)
{
    m_scale = { uniformScale, uniformScale, uniformScale };
    OnTransformChanged();
    return *this;
}

void Component::Transform::Reflect(TwBar* editor, std::string const& /*barName*/, std::string const& groupName,
    Graphics::GraphicsEngine* graphics)
{
    std::string defStr = "group="+ groupName;

    TwAddVarCB(editor, "Position", TW_TYPE_POINT(3), TwCallBack::SetPosition, TwCallBack::GetPosition, this, defStr.c_str());
    TwAddVarCB(editor, "Rotation Euler", TW_TYPE_POINT(3, 0.01f, "X degree", "Y degree", "Z degree"), TwCallBack::SetRotation, TwCallBack::GetRotation, this, (defStr + " help='degree'").c_str());
    TwAddVarCB(editor, "Scale", TW_TYPE_POINT(3), TwCallBack::SetScale, TwCallBack::GetScale, this, (defStr).c_str());
}


void Component::Transform::SetLocalTransform(Math::Matrix4 const& localTrans)
{
    m_localTransform = localTrans;
}

void Component::Transform::SetWorldTransform(Math::Matrix4 const& worldTrans)
{
    m_worldTransform = worldTrans;
}

Math::Matrix4 Component::Transform::CalcLocalTransform()
{
    //TODO(Assignment 1): Implement this function
    //Local transform is used to transform an object to world if the object does not have a
    //parent object. You don't need to worry about the world transformation for child objects
    //since it's handled by the framework in a hirarchical manner. You are very welcome to 
    //take a look at how world transform system works in Scene.h and HierarchicalObjectHandler.h

    //For the purpose of the first assignment, you only need to calculate the Euler rotation matrix
    //then call a function Math::Matrix4::BuildTransform() to build transform.

#if SAMPLE_IMPLEMENTATION
    Math::Matrix3 rotMatX;
    Math::Matrix3 rotMatY;
    Math::Matrix3 rotMatZ;
    rotMatX.Rotate(Math::Vector3::cXAxis, m_rotationEuler.x);
    rotMatY.Rotate(Math::Vector3::cYAxis, m_rotationEuler.y);
    rotMatZ.Rotate(Math::Vector3::cZAxis, m_rotationEuler.z);

    Math::Matrix3 rotMat = rotMatZ* rotMatY*rotMatX;    
    
    m_localTransform.BuildTransform(
        m_position,
        rotMat,
        m_scale
    );
#else
    m_localTransform.SetIdentity();    
    m_localTransform[2][3] = -2;//Hardcode position.z so you can see the model
#endif // SAMPLE_IMPLEMENTATION
    return m_localTransform;
}

void Component::Transform::OnTransformChanged()
{
    CalcLocalTransform();
    if (m_owner)
    {
        m_owner->GetScene()->InitTransformTree(m_owner->GetHierarchicalObjectHandlerNode());
        return;
        //BUG: rotation does not perform correctly
        //UpdateTransformTree needs to be fixed.
        Object* parent = m_owner->GetParent();
        if (parent)
        {
            m_owner->GetScene()->UpdateTransformTree(
                m_owner->GetHierarchicalObjectHandlerNode(), parent->GetComponentRef<Transform>().GetLocalTransform());
        }
        else
        {
            m_owner->GetScene()->UpdateTransformTree(
                m_owner->GetHierarchicalObjectHandlerNode(), Math::Matrix4::c_Identity);
        }
    }
    else
    {
        SetWorldTransform(m_localTransform);
    }
}
