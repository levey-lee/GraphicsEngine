/************************************************************************ 
*  @file Main.cpp
*  @brief Framework for CS300 at DigiPen
*  @author Zhengyang Li
*  @date December 2017
*  @coypright  Copyright (C) 2017-2018 DigiPen Institute of Technology.
***********************************************************************/

#include "Precompiled.h"
#include "framework/Application.h"
#include "framework/Debug.h"
#include "graphics/ShaderManager.h"
#include "graphics/TriangleMesh.h"
#include "graphics/MeshManager.h"
#include "math/Math.h"
#include "core/Scene.h"
#include "core/components/Light.h"
#include "core/components/Transform.h"
#include "core/components/Renderer.h"
#include "graphics/Materials.h"
#include "core/components/Camera.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/TextureManager.h"
#include "core/TwImpl.h"
#include "graphics/MaterialManager.h"
#include "core/components/Skydome.h"
#include "graphics/Texture.h"
#include "graphics/FramebufferManager.h"
#include "graphics/Framebuffer.h"
#include "graphics/ShaderProgram.h"
#ifdef _WIN32
#include <Windows.h>//for raw input so we can have a better camera control
#endif // _WIN32

static const unsigned c_DefaultWindowWidth = 1280;
static const unsigned c_DefaultWindowHeight = 760;
using namespace Graphics;
using namespace Math;

Scene g_MainScene;
std::shared_ptr<GraphicsEngine>g_Graphics;
ObjectHandle g_Obj0;
ObjectHandle g_Cam;
struct
{
    Vec2 mouseDragStartPoint;
    Vec2 mouseDragEndPoint;
    bool rightMouseDrag = false;

    Vec2 GetNormalizedDir() const
    {
        Vec2 dir = mouseDragEndPoint - mouseDragStartPoint;
        dir.y *= -1.0f;
        dir.AttemptNormalize();
        return dir;
    }
}g_MouseDragEventData;

//**************************************************************************
void Initialize(Application* app, void* /*userdata*/)
{
	TwInit(TW_OPENGL, nullptr);
	
    using namespace Component;
    g_Graphics = std::make_shared<GraphicsEngine>();
    g_Graphics->Initialize();

    std::shared_ptr<FramebufferManager> fboManager = g_Graphics->GetFrameBufferManager();
    fboManager->RegisterFramebuffer(FramebufferType::DeferredGBuffer, app->GetWindowWidth(), app->GetWindowHeight())->Build(FBO_USAGE_REGULAR);

    fboManager->RegisterFramebuffer(FramebufferType::GenShadowMap,512, 512)->Build(FBO_USAGE_FLOAT_BUFFER);
    fboManager->RegisterFramebuffer(FramebufferType::ShadowBlurH, 512, 512)->Build(FBO_USAGE_FLOAT_BUFFER);
    fboManager->RegisterFramebuffer(FramebufferType::ShadowBlurV, 512, 512)->Build(FBO_USAGE_FLOAT_BUFFER);

    fboManager->RegisterFramebuffer(FramebufferType::SSAO, app->GetWindowWidth(), app->GetWindowHeight())->Build(FBO_USAGE_DEPTH_BUFFER);
    fboManager->RegisterFramebuffer(FramebufferType::SSAOBlurH, app->GetWindowWidth(), app->GetWindowHeight())->Build(FBO_USAGE_DEPTH_BUFFER);
    fboManager->RegisterFramebuffer(FramebufferType::SSAOBlurV, app->GetWindowWidth(), app->GetWindowHeight())->Build(FBO_USAGE_DEPTH_BUFFER);

    
    ////////////////////////////////////////////////////////////////////////////
    //      Create meshes
    std::shared_ptr<MeshManager> meshManager = g_Graphics->GetMeshManager();


    meshManager->TriangleMeshHandler.LoadAndBuildObjMeshMultiThread(
    {
        { "sponge" ,        "menger_sponge_level_1_high_poly.obj" , DefaultUvType::Box },
        { "sphere" ,        "sphere.obj" , DefaultUvType::Spherical },
        { "sphereReversed", "sphereReversed.obj" , DefaultUvType::Spherical },
        { "teapot" ,        "teapot.obj" , DefaultUvType::Spherical },
        { "cube" ,          "cube.obj" , DefaultUvType::Box },
        { "plane" ,         "plane_low_poly.obj" , DefaultUvType::Box },
        { "bunny" ,         "bunny.obj" , DefaultUvType::Box },
        { "horse" ,         "horse_high_poly.obj" , DefaultUvType::Box },
        { "lucy_princeton" ,"lucy_princeton.obj" , DefaultUvType::Box },
    });

    std::shared_ptr<Mesh> spongeMesh = meshManager->GetMesh("sponge");
    //--------------------------------------
    std::shared_ptr<Mesh> sphereMesh = meshManager->GetMesh("sphere");
    //--------------------------------------
    std::shared_ptr<Mesh> sphereReversedMesh = meshManager->GetMesh("sphereReversed");
    //--------------------------------------
    std::shared_ptr<Mesh> teapotMesh = meshManager->GetMesh("teapot");
    //--------------------------------------
    std::shared_ptr<Mesh> cube = meshManager->GetMesh("cube");
    //--------------------------------------
    std::shared_ptr<Mesh> planeMesh = meshManager->GetMesh("plane");
    //--------------------------------------
    std::shared_ptr<Mesh> lucyMesh = meshManager->GetMesh("lucy_princeton");
    //--------------------------------------

    //------------------------------------------------------------------------------
    //std::shared_ptr<TriangleMesh> lucyMesh = meshManager->TriangleMeshHandler.LoadObjMesh("lucy_princeton", "lucy_princeton.obj", DefaultUvType::Box);
    //triangleMesh->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> triangleMesh = meshManager->TriangleMeshHandler.BuildTriangle("SampleTriangle");
    triangleMesh->Build();
    //------------------------------------------------------------------------------
    meshManager->TriangleMeshHandler.BuildFullScreenQuad("FSQ")->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> bTR80AMesh = meshManager->TriangleMeshHandler.LoadObjMeshWithUvNormal("BTR80A","BTR80A.obj");
    bTR80AMesh->Build();
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> golfMesh = meshManager->TriangleMeshHandler.LoadObjMeshWithUvNormal("Golf","golfball_high_poly.obj");
    golfMesh->CalcUvSpherical()->Build();
    //------------------------------------------------------------------------------

    ////////////////////////////////////////////////////////////////////////////
    //      Shader
    std::shared_ptr<ShaderManager> shaderManager = g_Graphics->GetShaderManager();
#if DEFERRED_SHADING_TEST
    ShaderType usingShader = ShaderType::UberDeferred;
    shaderManager->LoadShader(usingShader,
    {
        { "GBufferPass.vert", "GBufferPass.frag", ShaderUsage::RegularVSPS },
        { "GenAOFactor.vert", "GenAOFactor.frag", ShaderUsage::RegularVSPS },
        { "GenShadowMap.vert", "GenShadowMap.frag" , ShaderUsage::LightShadowMap },
        { "BlurSSAO.vert", "BlurSSAO.frag" , ShaderUsage::RegularVSPS },
        { "BlurShadowMap.vert", "BlurShadowMap.frag" , ShaderUsage::RegularVSPS },
        { "FinalPass.vert", "FinalPass.frag", ShaderUsage::RegularVSPS }
    });
#else
    ShaderType usingShader = ShaderType::UberForward;
    shaderManager->LoadShader(usingShader, "shader.vert", "shader.frag");
#endif // DEFERRED_SHADING_TEST


    //add materials to manager
    std::shared_ptr<MaterialManager> materialManager = g_Graphics->GetMaterialManager();
    materialManager->LoadMaterials("basic.mtl", g_Graphics);

    ////////////////////////////////////////////////////////////////////////////
    //      Create scene objects
    {
        //objects

        Object& BTR80A = g_MainScene.CreateObject(usingShader);
        BTR80A.AddComponent<Renderer>(materialManager->GetMaterial("BTR80A"), bTR80AMesh);
        BTR80A.GetComponentRef<Component::Transform>().SetPosition({ 0,0,-2 }).SetScale({ 1,1,1 }).SetRotation({ 0,0,0 });
        g_Obj0 = BTR80A.GetHandle();
        BTR80A.SetName("BTR80A");

        Object& plane = g_MainScene.CreateObject(usingShader);
        plane.AddComponent<Renderer>(materialManager->GetMaterial("Plane"), planeMesh);
        plane.GetComponentRef<Component::Transform>().SetPosition({ 5,-0.48f,-5 }).SetScale({ 0.03f,0.01f,0.02f }).SetRotation({ 0,0,0 });
        plane.SetName("Plane");

        Object& golfBall = g_MainScene.CreateObject(usingShader);
        golfBall.AddComponent<Renderer>(materialManager->GetMaterial("Golf"), golfMesh);
        golfBall.GetComponentRef<Component::Transform>().SetPosition({ 1,0,-5 }).SetScale(1).SetRotation({ 0,0,0 });
        golfBall.SetName("Golf");


        Object& teapotObj = g_MainScene.CreateObject(usingShader);
        teapotObj.AddComponent<Renderer>(materialManager->GetMaterial("Teapot"), teapotMesh);
        teapotObj.GetComponentRef<Component::Transform>().SetPosition({ 2, 0,-2 }).SetScale(1).SetRotation({ 0,-2.4f,0 });
        teapotObj.SetName("Teapot");


        Object& reversedSphere = g_MainScene.CreateObject(usingShader);
        reversedSphere.AddComponent<Renderer>(materialManager->GetMaterial("ReversedSphere"), sphereReversedMesh);
        reversedSphere.GetComponentRef<Component::Transform>().SetPosition({ 3,-0.18f,-5 }).SetScale(0.6f).SetRotation({ 0,0,0 });
        reversedSphere.SetName("ReversedSphere");


        Object& lucy = g_MainScene.CreateObject(usingShader);
        lucy.AddComponent<Renderer>(materialManager->GetMaterial("Lucy"), lucyMesh);
        lucy.GetComponentRef<Component::Transform>().SetPosition({ 3,0.26f,-0.1f }).SetScale(0.5f).SetRotation({ 0,0,0 });
        lucy.SetName("Lucy");

        Object& sphere = g_MainScene.CreateObject(usingShader);
        sphere.AddComponent<Renderer>(materialManager->GetMaterial("Sphere"), sphereMesh);
        sphere.GetComponentRef<Component::Transform>().SetPosition({ 5,-0.07f,-5 }).SetScale(0.8f).SetRotation({ 0,0,0 });
        sphere.SetName("Sphere");
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        Object& camObj = g_MainScene.CreateObject(usingShader);
        Component::Transform& camtrans = camObj.GetComponentRef<Component::Transform>();
        camtrans.SetPosition({2,2.5f,3});
        camtrans.SetRotation({ 0,0,0 });
        camtrans.SetScale(1000.0f);
        camObj.AddComponent<Camera>(camtrans, true, g_Graphics.get()).SetFieldOfViewDegree(90.0f);
        camObj.GetComponentRef<Camera>().RotateCameraLocal({ -0.45f,0,0 });
        camObj.GetComponentRef<Camera>().SetNearPlaneDistance(1.0f);
        camObj.AddComponent<Skydome>(materialManager->GetMaterial("Skydome"), sphereReversedMesh);
        //camObj.AddComponent<Light>()
        //    .SetLightType(LightType::Directional)
        //    ->SetAmbientColor(Color(0.1f, 0.1f, 0.1f))
        //    ->SetSpecularColor(Color(0.2f, 0.2f, 0.2f))
        //    ->SetShadowType(ShadowType::HardShadow);
        camObj.SetName("Camera");
        g_Cam = camObj.GetHandle();


        Object& lightObj = g_MainScene.CreateObject(usingShader);
        Component::Transform& lightTrans = lightObj.GetComponentRef<Component::Transform>();
        lightTrans.SetPosition({-8,10,-3.5f}).SetRotation({ -c_Pi / 2.0f, 0, c_Pi / 4.0f });
        lightObj.AddComponent<Camera>(lightTrans, false, g_Graphics.get()).SetFieldOfViewDegree(45.0f);
        lightObj.GetComponentRef<Camera>().SetNearPlaneDistance(1.0f);
        lightObj.AddComponent<Light>()
            .SetLightType(LightType::Spot)
            ->SetAmbientColor(Color(0,0,0))
            ->SetSpecularColor(Color(0.8f, 0.8f, 0.8f))
            ->SetDistanceAttenuation(0.01f,0.01f,0)
            ->SetShadowType(ShadowType::SoftShadow)
	        ->SetSpotlightFalloff(8.0f);
        lightObj.SetName("Light");

    }
    TwEditor::CreateComponentEditor("Object & Component", g_MainScene.GetEditorObjectRef(), g_Graphics);
    TwEditor::CreateResourceEditor("Resource Manager", g_Graphics->GetTextureManager()->GetAllTextures(), materialManager->GetAllMaterials(), g_Graphics);
    

    g_MainScene.StartScene();
}

//**************************************************************************
void Update(Application* /*application*/, float dt, void* /*userdata*/)
{
    g_MainScene.UpdateScene(dt);
    g_Graphics->RenderScene(&g_MainScene);
#ifdef _WIN32//for a better keyboard input
    static const float CameraMoveSpeed = 0.05f;
    Object& camObj = g_MainScene.GetObjectRef(g_Cam);
    Component::Camera& cam = camObj.GetComponentRef<Component::Camera>();
    Component::Transform& camWorldTrans = camObj.GetComponentRef<Component::Transform>();
    Vector3 const& viewVec = cam.GetViewVector();
    Vector3 const& upVec = cam.GetUpVector();

    if (GetAsyncKeyState('W') )
    {
        camWorldTrans.Translate(viewVec*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('S'))
    {
        camWorldTrans.Translate(-viewVec*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('Q'))
    {
        Vec3 leftDir = Cross(upVec, viewVec);
        camWorldTrans.Translate(leftDir*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('E'))
    {
        Vec3 rightDir = Cross(viewVec, upVec);
        camWorldTrans.Translate(rightDir*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('A'))
    {
        cam.RotateCameraLocal(upVec*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('D'))
    {
        cam.RotateCameraLocal(-upVec*CameraMoveSpeed);
    }
    if (GetAsyncKeyState(VK_SPACE))
    {
        camWorldTrans.Translate(upVec*CameraMoveSpeed);
    }
    if (GetAsyncKeyState(VK_CONTROL))
    {
        camWorldTrans.Translate(-upVec*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('Z'))
    {
        Vec3 rightDir = Cross(viewVec, upVec);
        cam.RotateCameraLocal(rightDir*CameraMoveSpeed);
    }
    if (GetAsyncKeyState('X'))
    {
        Vec3 leftDir = Cross(upVec, viewVec);
        cam.RotateCameraLocal(leftDir*CameraMoveSpeed);
    }
#endif // _WIN32

	TwDraw();
}

//**************************************************************************
void Loading(Application* application, float dt, void* userdata)
{
    std::shared_ptr<TextureManager> textureManager = g_Graphics->GetTextureManager();
    size_t remainingJobs = textureManager->ProcessThreadLoadedTexture();
    Update(application, dt, userdata);
    if (remainingJobs == 0)
    {
        application->SetUpdateFunc(Update);
    }
}
//**************************************************************************
void Cleanup(Application* /*application*/, void* /*udata*/)
{
    TwDeleteAllBars();
	TwTerminate();
}

//**************************************************************************
void OnViewportChanged(Application* application)
{    
    g_Graphics->GetViewCamera()->SetDimension(
        static_cast<float>(application->GetWindowWidth()),
        static_cast<float>(application->GetWindowHeight()));
    auto fboManager = g_Graphics->GetFrameBufferManager();
    fboManager->GetFramebuffer(FramebufferType::DeferredGBuffer)->Resize(
        application->GetWindowWidth(), application->GetWindowHeight(),false);
    
    fboManager->GetFramebuffer(FramebufferType::SSAO)->Resize(
        application->GetWindowWidth(), application->GetWindowHeight(), false);
    fboManager->GetFramebuffer(FramebufferType::SSAOBlurH)->Resize(
        application->GetWindowWidth(), application->GetWindowHeight(), true);
    fboManager->GetFramebuffer(FramebufferType::SSAOBlurV)->Resize(
        application->GetWindowWidth(), application->GetWindowHeight(), true);

}

//**************************************************************************
void OnMouseWheel(Application* /*application*/, int /*wheel*/, int direction, int /*x*/, int /*y*/)
{
    float fov = g_Graphics->GetViewCamera()->GetFieldOfViewRadians();
    if (direction > 0)
    {
        if (fov > 0.03f)
        {
            g_Graphics->GetViewCamera()->SetFieldOfViewRadians(fov - static_cast<float>(direction) * 0.015f);
        }
    }
    else if (fov < c_Pi)
    {
        g_Graphics->GetViewCamera()->SetFieldOfViewRadians(fov - static_cast<float>(direction) * 0.015f);
    }
    else
    {
        fov = c_Pi-0.1f;
    }
}
void OnMouseButtonDown(Application* application, int button, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        Ray selectionRay = application->GetRayFromScreenCoords(x, y,
            g_Graphics->GetViewCamera()->GetCameraWorldPosition(),
            g_Graphics->GetViewCamera()->GetViewMatrix(),
            g_Graphics->GetViewCamera()->GetProjMatrix()
        );

        std::vector<Object*> const& editorObjects = g_MainScene.GetEditorObjectRef();

        bool hit = false;
        float t = -1;
        float nearestT = FLT_MAX;
        Object* obj = nullptr;
        for (auto& i : editorObjects)
        {
            BoundingSphere sphere = i->GetBoundingSphere();
            hit = selectionRay.CheckCollisionSphere(sphere.center, sphere.radius, &t);
            if (hit && (t < nearestT))
            {
                nearestT = t;
                obj = i;
            }
        }
        if (obj)
        {
            TwEditor::SetSelection(obj);
        }
        else
        {
            TwEditor::SetSelection(nullptr);
        }
    }
    else if (button == GLUT_RIGHT_BUTTON)
    {
        g_MouseDragEventData.rightMouseDrag = true;
        g_MouseDragEventData.mouseDragStartPoint = Vec2(float(x), float(y));
    }
}
void OnMouseButtonUp(Application* application, int button, int x, int y)
{
    if (button == GLUT_RIGHT_BUTTON)
    {
        g_MouseDragEventData.rightMouseDrag = false;
    }
}

void OnMouseDrag(Application* , int x, int y)
{
    static const float CameraRotateSpeed = 0.05f;
    if (g_MouseDragEventData.rightMouseDrag)
    {
        Object& camObj = g_MainScene.GetObjectRef(g_Cam);
        Component::Camera& cam = camObj.GetComponentRef<Component::Camera>();
        Component::Transform& camWorldTrans = camObj.GetComponentRef<Component::Transform>();
        Vec3 const& viewVec = cam.GetViewVector();
        Vec3 const& upVec = cam.GetUpVector();
        Vec3 rightDir = Cross(viewVec, upVec);

        g_MouseDragEventData.mouseDragEndPoint = Vec2(float(x), float(y));
        Vec2 rightUp = g_MouseDragEventData.GetNormalizedDir();
        camWorldTrans.Translate(rightDir*CameraRotateSpeed*rightUp.x);
        camWorldTrans.Translate(upVec*CameraRotateSpeed*rightUp.y);

        g_MouseDragEventData.mouseDragStartPoint = Vec2(float(x), float(y));
    }
}
void SimpleKeyDonwCB(Application* , unsigned char key, int x, int y)
{
#ifndef _WIN32
    static const float CameraMoveSpeed = 0.05f;
    Object& camObj = g_MainScene.GetObjectRef(g_Cam);
    Component::Camera& cam = camObj.GetComponentRef<Component::Camera>();
    Component::Transform& camWorldTrans = camObj.GetComponentRef<Component::Transform>();
    Vec3 const& viewVec = cam.GetViewVector();
    Vec3 const& upVec = cam.GetUpVector();

    if (key == 'w')
    {
        camWorldTrans.Translate(viewVec*CameraMoveSpeed);
    }
    if (key == 's')
    {
        camWorldTrans.Translate(-viewVec*CameraMoveSpeed);
    }
    if (key == 'q')
    {
        Vec3 leftDir = Cross(upVec, viewVec);
        camWorldTrans.Translate(leftDir*CameraMoveSpeed);
    }
    if (key == 'e')
    {
        Vec3 rightDir = Cross(viewVec, upVec);
        camWorldTrans.Translate(rightDir*CameraMoveSpeed);
    }
    if (key == 'a')
    {
        cam.RotateCameraLocal(upVec*CameraMoveSpeed);
    }
    if (key == 'd')
    {
        cam.RotateCameraLocal(-upVec*CameraMoveSpeed);
    }
    if (key == 'z')
    {
        camWorldTrans.Translate(upVec*CameraMoveSpeed);
    }
    if (key == 'x')
    {
        camWorldTrans.Translate(-upVec*CameraMoveSpeed);
    }

#endif // !_WIN32


}

//**************************************************************************
int main(int argc, char* argv[])
{
    Application* app = &Application::GetInstance();
    app->Initialize(argc, argv, "Diamond Graphics", c_DefaultWindowWidth, c_DefaultWindowHeight);
    app->SetOnViewportChanged(OnViewportChanged);
    app->SetMouseWheelCallback(OnMouseWheel);
    app->SetMouseButtonDownCallback(OnMouseButtonDown);
    app->SetMouseButtonUpCallback(OnMouseButtonUp);
    app->SetMouseDragCallback(OnMouseDrag);
    app->SetKeyDownCallback(SimpleKeyDonwCB);
    app->Run(Initialize, Loading, Cleanup);
    return 0;
}
