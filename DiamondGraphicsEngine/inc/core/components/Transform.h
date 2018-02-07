#pragma once
#include "core/ComponentBase.h"
#include "math/Vector3.h"
#include "math/Matrix4.h"

namespace Component
{
    /*******************************************************
     * @brief Transform for all object. Be aware that every
     * object has a transform component by default.
     * @note Transform does not call Update even if it changes.
     * It calls a event callback function to update hirarchical 
     * transform tree in the scene.
     *******************************************************/
    class Transform
        : public ComponentBase<Transform, SHADED>
    {

        friend class Scene;//to set local & world transform
    public:
        Transform(Graphics::ShaderType shaderType) : ComponentBase(true, shaderType) {}
        ~Transform() = default;

        void SetShaderParams(std::shared_ptr<Graphics::ShaderProgram> shader, Graphics::GraphicsEngine* graphics) override;

        Transform& Translate(Math::Vector3 const& trans);
        Transform& Scale(Math::Vector3 const& scale);
        Transform& Scale(float uniformScale);
        Transform& Rotate(Math::Vector3 const& angleRadianXYZ);
        Transform& Orbit(Math::Vector3 const& axis, float angleRadian);

        Math::Vector3 const& GetPosition() const { return m_position; }
        Math::Vector3 const& GetRotationEuler() const { return m_rotationEuler; }
        Math::Vector3 const& GetScale() const { return m_scale; }

        Math::Matrix4 const& GetLocalTransform() const;
        Math::Matrix4 const& GetWorldTransform() const;
        
        Transform& SetPosition(Math::Vector3 const& pos);
        Transform& SetRotation(Math::Vector3 const& rotEuler);
        Transform& SetScale(Math::Vector3 const& scale);
        Transform& SetScale(float uniformScale);

        Math::Matrix4 CalcLocalTransform();

        REGISTER_EDITOR_COMPONENT(Transform)
		void Reflect(TwBar* editor, std::string const& barName, std::string const& groupName, Graphics::GraphicsEngine* graphics) override;

    protected:

        void SetLocalTransform(Math::Matrix4 const& localTrans);
        void SetWorldTransform(Math::Matrix4 const& worldTrans);

        // set self and children's world
        void OnTransformChanged();
    
    private:

        //local position
        Math::Vector3 m_position = {0,0,0};

        //local rotation
        Math::Vector3 m_rotationEuler = {0,0,0};

        //local scale
        Math::Vector3 m_scale = {1,1,1};

        Math::Matrix4 m_localTransform;
        Math::Matrix4 m_worldTransform;
    };
}

