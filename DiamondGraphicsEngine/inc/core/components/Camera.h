#pragma once
#include "core/ComponentBase.h"
#include "graphics/CameraBase.h"

namespace Component
{
    class Transform;

    class Camera
        : public ComponentBase<Camera, UNSHADED>
          , public Graphics::CameraBase
    {
    public:
        Camera() = delete;

        /*******************************************************
         * @brief Camera component constructor
         * @param trans Reference to transform class to detect if transform changed
         * @param defaultEnable If set this camera to be the view camera
         * @param graphics 
         *******************************************************/
        Camera(Transform& trans, bool defaultEnable = false, Graphics::GraphicsEngine* graphics = nullptr);
        ~Camera();

        //////////////////////////////////////////////////
        /// Component methods
        void Start() override;
        void Update(float dt) override;
        void StaticUpdate(float dt) { std::cout << "Camera Static Update\n"; }
        void Disable() override;


        //////////////////////////////////////////////////
        /// Camera methods
        void CalcViewMatrix() override;
        Math::Vector3 const& CalcViewVector() override;
        Math::Vector3 const& CalcUpVector() override;
        Math::Vector3 GetCameraLocalPosition() override { return m_position; }
        Math::Vector3 GetCameraWorldPosition() override;
        Math::Vector3 GetCameraLocalRotationEuler() override { return m_rotation; }
        Math::Vector3 GetCameraWorldRotationEuler() override;
        void RotateCameraLocal(Math::Vector3 const& xyzRad)override;

		//////////////////////////////////////////////////
		/// Editor
        REGISTER_EDITOR_COMPONENT(Camera)
		void Reflect(TwBar* editor, std::string const& barName, std::string const& groupName,
            Graphics::GraphicsEngine* graphics) override;
                
    protected:
		void OnEnable() override;
    private:
        Transform& m_transform;
        Math::Vector3 m_position;
        Math::Vector3 m_rotation;
        Math::Vector3 m_scale;
        Graphics::GraphicsEngine* m_graphics = nullptr;
    };
}
