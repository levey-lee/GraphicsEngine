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
#include "graphics/FramebufferManager.h"
#include "framework/Application.h"
#include "graphics/Framebuffer.h"

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
        m_frameBufferManager = std::make_shared<FramebufferManager>(&Application::GetInstance());

        SetBackgroundColor(Color(0.1f,0.1f,0.1f));
        EnableDepthTest();
        glCullFace(GL_BACK);
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

    Math::Matrix4 GraphicsEngine::GetLightViewProj()
    {
        return m_lightManager->GetLightViewProj();
    }
    Math::Vec3 GraphicsEngine::GetShadowingLightPos()
    {
        return m_lightManager->GetShadowingLightPos();
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

    void GraphicsEngine::deferredRender(const std::shared_ptr<Shader>& shader, std::unordered_map<ObjectId, RenderObject*>& obj)
    {
        
        
        //TODO Deferred Shading Step 1 : fill framebuffer with multiple attachments(GBuffer)
        std::shared_ptr<ShaderProgram> program = shader->GetShaderProgram(ShaderStage::DiffuseMaterial);
        program->Bind();
        EnableDepthTest();
        m_frameBufferManager->Bind(FramebufferType::DeferredGBuffer);
        m_frameBufferManager->Clear(FramebufferType::DeferredGBuffer);

        for (auto& j : obj)//per object
        {
            for (auto& k : *j.second)//per shaded component
            {
                k->SetShaderParams(program, this);
            }
            m_textureManager->UnbindAll();
        }

        //TODO Deferred Shading Step 2 : Generate SSAO factor
        program = shader->GetShaderProgram(ShaderStage::SSAO);
        program->Bind();
        m_frameBufferManager->Bind(FramebufferType::SSAO);
        m_frameBufferManager->Clear(FramebufferType::SSAO);
        std::shared_ptr<Framebuffer> fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::DeferredGBuffer);
        fbo->BindGBufferPositionNormal(program);
        fbo->BindDepthTexture(program);

        float screenWidth = static_cast<float>(Application::GetInstance().GetWindowWidth());
        float screenHeight = static_cast<float>(Application::GetInstance().GetWindowHeight());
        program->SetUniform("ScreenDimension", Math::Vec2(screenWidth, screenHeight));
        program->SetUniform("ControlVariable", SSAO.ControlVariable);
        program->SetUniform("SamplePointNum", SSAO.SamplePointNum);
        program->SetUniform("RangeOfInfluence", SSAO.RangeOfInfluence);


        m_meshManager->GetMesh("FSQ")->Render();


        //TODO Deferred Shading Step 3 : Blur SSAO map Horinzontally
        program = shader->GetShaderProgram(ShaderStage::SSAOBlurH);
        program->Bind();

        m_frameBufferManager->Bind(FramebufferType::SSAOBlurH);
        m_frameBufferManager->Clear(FramebufferType::SSAOBlurH);
        m_frameBufferManager->GetFramebuffer(FramebufferType::SSAO)->BindSSAOTexture(program, false);

        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::DeferredGBuffer);
        fbo->BindGBufferNormal(program);
        fbo->BindDepthTexture(program);

        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::SSAO);
        float fboWidth = static_cast<float>(fbo->GetWidth());
        float fboHeight = static_cast<float>(fbo->GetHeight());
        program->SetUniform("BlurWidth", SSAO.BlurWidth);
        program->SetUniform("EdgeStrength", SSAO.EdgeStrength);
        program->SetUniform("ScreenDimension", Math::Vec2(fboWidth, fboHeight));

        m_meshManager->GetMesh("FSQ")->Render();


        //TODO Deferred Shading Step 4 : Blur SSAO map Vertically
        program = shader->GetShaderProgram(ShaderStage::SSAOBlurV);
        program->Bind();

        m_frameBufferManager->Bind(FramebufferType::SSAOBlurV);
        m_frameBufferManager->Clear(FramebufferType::SSAOBlurV);
        m_frameBufferManager->GetFramebuffer(FramebufferType::SSAOBlurH)->BindSSAOTexture(program, true);

        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::DeferredGBuffer);
        fbo->BindGBufferNormal(program);
        fbo->BindDepthTexture(program);

        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::SSAOBlurH);
        fboWidth = static_cast<float>(fbo->GetWidth());
        fboHeight = static_cast<float>(fbo->GetHeight());
        program->SetUniform("BlurWidth", SSAO.BlurWidth);
        program->SetUniform("EdgeStrength", SSAO.EdgeStrength);
        program->SetUniform("ScreenDimension", Math::Vec2(fboWidth, fboHeight));
        m_meshManager->GetMesh("FSQ")->Render();

        //TODO Deferred Shading Step 5 : Generate Shadow Map
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT);
        program = shader->GetShaderProgram(ShaderStage::DeferredLighting);
        program->Bind();
        m_frameBufferManager->Bind(FramebufferType::DeferredShadowMap);
        m_frameBufferManager->Clear(FramebufferType::DeferredShadowMap);
        m_lightManager->SetLightShadowUniforms(program);
        //m_viewCamera->SetCameraUniforms(program);
        for (auto& i : obj)//per object
        {
            for (auto& j : *i.second)//per shaded component
            {
                j->SetShaderParams(program, this);
            }
        }
        DisableDepthTest();
        //glDisable(GL_CULL_FACE);

        //TODO Deferred Shading Step 6 : Blur ShadowMap Horinzontally
        program = shader->GetShaderProgram(ShaderStage::ShadowBlurH);
        program->Bind();
        m_lightManager->SetShadowFilterUniforms(program);
        m_frameBufferManager->Bind(FramebufferType::ShadowBlurH);
        m_frameBufferManager->Clear(FramebufferType::ShadowBlurH);
        m_frameBufferManager->GetFramebuffer(FramebufferType::DeferredShadowMap)->BindShadowMapTexture(program);

        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::ShadowBlurH);
        fboWidth = static_cast<float>(fbo->GetWidth());
        fboHeight = static_cast<float>(fbo->GetHeight());
        program->SetUniform("ScreenDimension", Math::Vec2(fboWidth, fboHeight));
        m_meshManager->GetMesh("FSQ")->Render();


        //TODO Deferred Shading Step 7 : Blur ShadowMap Vertically
        program = shader->GetShaderProgram(ShaderStage::ShadowBlurV);
        program->Bind();
        m_lightManager->SetShadowFilterUniforms(program);
        m_frameBufferManager->Bind(FramebufferType::ShadowBlurV);
        m_frameBufferManager->Clear(FramebufferType::ShadowBlurV);
        m_frameBufferManager->GetFramebuffer(FramebufferType::ShadowBlurH)->BindShadowMapTexture(program);

        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::ShadowBlurV);
        fboWidth = static_cast<float>(fbo->GetWidth());
        fboHeight = static_cast<float>(fbo->GetHeight());
        program->SetUniform("ScreenDimension", Math::Vec2(fboWidth, fboHeight));
        m_meshManager->GetMesh("FSQ")->Render();
        

        //TODO Deferred Shading Step 8 : Combine everything
        m_frameBufferManager->Bind(FramebufferType::Screen);
        program = shader->GetShaderProgram(ShaderStage::RenderFullScreenQuad);
        program->Bind();

        //shadow map
        m_frameBufferManager->GetFramebuffer(FramebufferType::ShadowBlurV)->BindShadowMapTexture(program);
        program->SetUniform("LightViewProj", m_lightManager->GetLightViewProj());

        //light
        m_viewCamera->SetCameraUniforms(program);
        m_lightManager->SetLightsUniform(program);
        m_lightManager->SetLightShadowUniforms(program);
        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::DeferredGBuffer);
        fbo->BindGBufferTextures(program);
        fbo->BindDepthTexture(program);

        //ssao
        fbo = m_frameBufferManager->GetFramebuffer(FramebufferType::SSAOBlurV);
        fbo->BindSSAOTexture(program, true);

        program->SetUniform("DebugOutputIndex", DebugRenderUniform.OutputIndex);
        program->SetUniform("EnableBlur", DebugRenderUniform.EnableBlur);
        program->SetUniform("BlurStrength", DebugRenderUniform.BlurStrength);
        program->SetUniform("EnableSSAO", DebugRenderUniform.EnableSSAO);
        screenWidth = static_cast<float>(Application::GetInstance().GetWindowWidth());
        screenHeight = static_cast<float>(Application::GetInstance().GetWindowHeight());
        program->SetUniform("ScreenDimension", Math::Vec2(screenWidth, screenHeight));

        m_meshManager->GetMesh("FSQ")->Render();
        program->Validate();
    }
}


