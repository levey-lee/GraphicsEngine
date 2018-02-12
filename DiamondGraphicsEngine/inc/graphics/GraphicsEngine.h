#pragma once
#include "graphics/Color.h"
#include "graphics/Shader.h"
#include "core/Object.h"
#include "math/Matrix4.h"

class ComponentInterface;
class Scene;

namespace Graphics
{
    class MeshManager;
    class MaterialManager;
    class LightManager;
    class CameraBase;
    class ShaderManager;
    class TextureManager;
    class FramebufferManager;
    class GraphicsEngine
    {
    public:
        void Initialize();
        void RenderScene(Scene* scene);

        CameraBase* GetViewCamera() const { return m_viewCamera; }

        std::shared_ptr<TextureManager>     GetTextureManager()     const { return m_textureManager; }
        std::shared_ptr<ShaderManager>      GetShaderManager()      const { return m_shaderManager; }
        std::shared_ptr<LightManager>       GetLightManager()       const { return m_lightManager; }
        std::shared_ptr<MaterialManager>    GetMaterialManager()    const { return m_materialManager; }
        std::shared_ptr<MeshManager>        GetMeshManager()        const { return m_meshManager; }
        std::shared_ptr<FramebufferManager> GetFrameBufferManager() const { return m_frameBufferManager; }
        Color GetBackgroundColor() const { return m_backgroundColor; }
        /*******************************************************
         * @brief This sets a camera to be a view camera. It can be derived from
         * CameraBase. Only one camera will be active at the same time.
         * @param viewCam Pointer to a camera class.
         * @param camComp If you are passing camera from component, this must be the 
         * same as viewCam. nullptr otherwise.
         *******************************************************/
        void SetViewCamera(CameraBase* viewCam, ComponentInterface* camComp = nullptr);
        void SetBackgroundColor(Color const& color);
        void EnableDepthTest() const {glEnable(GL_DEPTH_TEST); }
        void DisableDepthTest() const { glDisable(GL_DEPTH_TEST); }

        Math::Matrix4 GetLightViewProj();

        struct
        {
            int OutputIndex = 0;
            int EnableBlur = 0;
            int BlurStrength = 0;
        }DebugRenderUniform;
    private:
        void renderScene(Scene* scene);
        void forwardRender(const std::shared_ptr<Shader>& shader, std::unordered_map<ObjectId, RenderObject*>& obj);
        void deferredRender(const std::shared_ptr<Shader>& shader, std::unordered_map<ObjectId, RenderObject*>& obj);

        Color m_backgroundColor;
        CameraBase* m_viewCamera = nullptr;
        ComponentInterface* m_viewCamComp = nullptr;
        
        std::shared_ptr<TextureManager>         m_textureManager;
        std::shared_ptr<ShaderManager>          m_shaderManager;
        std::shared_ptr<LightManager>           m_lightManager;
        std::shared_ptr<MaterialManager>        m_materialManager;
        std::shared_ptr<MeshManager>            m_meshManager;
        std::shared_ptr<FramebufferManager>     m_frameBufferManager;
    };
}

