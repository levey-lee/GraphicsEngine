#include "Precompiled.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/CameraBase.h"
#include "graphics/ShaderManager.h"
#include "core/Scene.h"
#include "core/ComponentBase.h"
#include "graphics/LightManager.h"
#include "graphics/ShaderProgram.h"
#include "graphics/TextureManager.h"
#include "graphics/MaterialManager.h"
#include "graphics/MeshManager.h"

namespace Graphics
{
    void GraphicsEngine::Initialize()
    {
        m_viewCamera = &CameraBase::DefaultCamera;
        m_viewCamera->CalcViewMatrix();
        m_viewCamera->CalcProjMatrix();
        m_viewCamera->CalcViewProjMatrix();
        m_shaderManager = std::make_shared<ShaderManager>();
        m_lightManager = std::make_shared<LightManager>();
        m_textureManager = std::make_shared<TextureManager>();
        m_materialManager = std::make_shared<MaterialManager>();
        m_meshManager = std::make_shared<MeshManager>();

        SetBackgroundColor(Color(0.1f,0.1f,0.1f));
        EnableDepthTest();
    }

    void GraphicsEngine::RenderScene(Scene* scene)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderScene(scene);
    }

    void GraphicsEngine::SetViewCamera(CameraBase* viewCam, ComponentInterface* camComp)
    {
        if (viewCam == nullptr)
        {
            Warning("No camera is set! Using default.");
            m_viewCamera = &CameraBase::DefaultCamera;
            m_viewCamComp = nullptr;
            return;
        }
        if (camComp)
        {
            Assert(camComp->IsEnabled(),
                "Camera component must be enabled before setting to view camera.");

            if (m_viewCamComp)
            {
                m_viewCamComp->SetEnabled(false);
            }
            m_viewCamComp = camComp;
        }
        m_viewCamera = viewCam;
    }

    void GraphicsEngine::SetBackgroundColor(Color const& color)
    {
        m_backgroundColor = color;
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void GraphicsEngine::renderScene(Scene* scene)
    {
        auto& renderList = scene->GetRenderObjectListRef();
        for (auto& i : renderList)//per shader
        {
            //bind this shader to render all object with this shader type
            std::shared_ptr<Shader> shader = m_shaderManager->GetShader(i.first);
            
            if (shader->IsDeferred())
            {
                deferredRender(shader, i.second);
            }
            else
            {
                forwardRender(shader, i.second);
            }
        }

        //finished setting up shader uniforms, unbind all
        m_shaderManager->UnbindAllShader();
    }

    void GraphicsEngine::forwardRender(const std::shared_ptr<Shader>& shader, std::unordered_map<ObjectId, RenderObject*>& obj)
    {
        std::shared_ptr<ShaderProgram> program = shader->GetShaderProgram(ShaderStage::ForwardRendering);
        program->Bind();
        m_viewCamera->SetCameraUniforms(program);

        m_lightManager->SetLightsUniform(program);
        for (auto& j : obj)//per object
        {
            for (auto& k : *j.second)//per shaded component
            {
                k->SetShaderParams(program, this);
            }
            m_textureManager->UnbindAll();
        }
    }

    void GraphicsEngine::deferredRender(std::shared_ptr<Shader> shader, std::unordered_map<ObjectId, RenderObject*>& obj)
    {
        std::shared_ptr<ShaderProgram> program = shader->GetShaderProgram(ShaderStage::DiffuseMaterial);
        program->Bind();
        //TODO Deferred Shading Step 1 : fill framebuffer with multiple attachments(GBuffer)

        for (auto& j : obj)//per object
        {
            for (auto& k : *j.second)//per shaded component
            {
                k->SetShaderParams(program, this);
            }
        }

        //TODO Deferred Shading Step 2 : Set Light and framebuffer textures
        program = shader->GetShaderProgram(ShaderStage::DeferredLighting);
        program->Bind();
        m_lightManager->SetLightsUniform(program);

        
        //TODO Deferred Shading Step 3 : Render FSQ
        program = shader->GetShaderProgram(ShaderStage::RenderFullScreenQuad);
        program->Bind();
    }
}


