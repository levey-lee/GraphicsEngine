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

static const unsigned c_DefaultWindowWidth = 1280;
static const unsigned c_DefaultWindowHeight = 760;
using namespace Graphics;
using namespace Math;

Scene g_MainScene;
std::shared_ptr<GraphicsEngine>g_Graphics;
ObjectHandle g_Sun;
#if SAMPLE_IMPLEMENTATION
ObjectHandle g_Earth;
ObjectHandle g_Mercury;
ObjectHandle g_Venus;
ObjectHandle g_Jupiter;
ObjectHandle g_Saturn;
ObjectHandle g_Uranus;
ObjectHandle g_Neptune;
ObjectHandle g_Moon;
ObjectHandle g_Cam;
ObjectHandle g_Mars;
#endif // SAMPLE_IMPLEMENTATION


//**************************************************************************
void Initialize(Application* app, void* /*userdata*/)
{
	TwInit(TW_OPENGL, nullptr);
	
    using namespace Component;
    g_Graphics = std::make_shared<GraphicsEngine>();
    g_Graphics->Initialize();

    //add materials to manager
    std::shared_ptr<MaterialManager> materialManager = g_Graphics->GetMaterialManager();
    materialManager->LoadMaterials("basic.mtl", g_Graphics);

    std::shared_ptr<FramebufferManager> fboManager = g_Graphics->GetFrameBufferManager();
    fboManager->RegisterFramebuffer(FramebufferType::DeferredGBuffer, app->GetWindowWidth(), app->GetWindowHeight())->Build();

    
    ////////////////////////////////////////////////////////////////////////////
    //      Create meshes
    std::shared_ptr<MeshManager> meshManager = g_Graphics->GetMeshManager();
    std::shared_ptr<TriangleMesh> lowSphereMesh = meshManager->TriangleMeshHandler.BuildSphere("lowPolySphere", DefaultUvType::Spherical);
    lowSphereMesh->Build();
#if SAMPLE_IMPLEMENTATION
#if 0

    std::shared_ptr<TriangleMesh> spongeMesh = meshManager->TriangleMeshHandler.LoadObjMesh("sponge", "menger_sponge_level_1_low_poly.obj", DefaultUvType::Box);
    spongeMesh->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> sphereMesh = meshManager->TriangleMeshHandler.LoadObjMesh("sphere", "sphere.obj", DefaultUvType::Spherical);
    sphereMesh->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> sphereReversedMesh = meshManager->TriangleMeshHandler.LoadObjMesh("sphereReversed", "sphereReversed.obj", DefaultUvType::Spherical);
    sphereReversedMesh->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> teapot = meshManager->TriangleMeshHandler.LoadObjMesh("teapot", "teapot.obj", DefaultUvType::Box);
    teapot->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> cube = meshManager->TriangleMeshHandler.LoadObjMesh("cube", "cube.obj", DefaultUvType::Box);
    cube->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> planeMesh = meshManager->TriangleMeshHandler.LoadObjMesh("plane", "plane_low_poly.obj", DefaultUvType::Box);
    planeMesh->Build();
#else
    meshManager->TriangleMeshHandler.LoadAndBuildObjMeshMultiThread(
    {
        { "sponge" ,        "menger_sponge_level_1_high_poly.obj" , DefaultUvType::Box },
        { "sphere" ,        "sphere.obj" , DefaultUvType::Spherical },
        { "sphereReversed", "sphereReversed.obj" , DefaultUvType::Spherical },
        { "teapot" ,        "teapot.obj" , DefaultUvType::Box },
        { "cube" ,          "cube.obj" , DefaultUvType::Box },
        { "plane" ,          "plane_low_poly.obj" , DefaultUvType::Box },
    });

    std::shared_ptr<Mesh> spongeMesh = meshManager->GetMesh("sponge");
    //--------------------------------------
    std::shared_ptr<Mesh> sphereMesh = meshManager->GetMesh("sphere");
    //--------------------------------------
    std::shared_ptr<Mesh> sphereReversedMesh = meshManager->GetMesh("sphereReversed");
    //--------------------------------------
    std::shared_ptr<Mesh> teapot = meshManager->GetMesh("teapot");
    //--------------------------------------
    std::shared_ptr<Mesh> cube = meshManager->GetMesh("cube");
    //--------------------------------------
    std::shared_ptr<Mesh> planeMesh = meshManager->GetMesh("plane");

#endif // 0
#endif // SAMPLE_IMPLEMENTATION
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> triangleMesh = meshManager->TriangleMeshHandler.BuildTriangle("SampleTriangle");
    triangleMesh->Build();
    //------------------------------------------------------------------------------
    meshManager->TriangleMeshHandler.BuildFullScreenQuad("FSQ")->Build();
    //------------------------------------------------------------------------------
    std::shared_ptr<TriangleMesh> tank = meshManager->TriangleMeshHandler.LoadObjMeshWithUvNormal("BTR80A","BTR80A.obj");
    tank->Build();
    //------------------------------------------------------------------------------

    ////////////////////////////////////////////////////////////////////////////
    //      Shader
    std::shared_ptr<ShaderManager> shaderManager = g_Graphics->GetShaderManager();
#if DEFERRED_SHADING_TEST
    ShaderType usingShader = ShaderType::UberDeferred;
    shaderManager->LoadShader(usingShader,
    {
        { "DeferredStage0.vert", "DeferredStage0.frag" },
        //{ "DeferredStage1.vert", "DeferredStage1.frag" },
        { "DeferredStage2.vert", "DeferredStage2.frag" }
    });
#else
    ShaderType usingShader = ShaderType::UberForward;
    shaderManager->LoadShader(usingShader, "shader.vert", "shader.frag");
#endif // DEFERRED_SHADING_TEST


    ////////////////////////////////////////////////////////////////////////////
    //      Create scene objects
    {
        //objects

        Object& sun = g_MainScene.CreateObject(usingShader);
        sun.AddComponent<Renderer>(materialManager->GetMaterial("Sun"), sphereMesh);
        sun.GetComponentRef<Component::Transform>().SetPosition({0,0,-2}).SetScale({1,1,1}).SetRotation({ 0.1f,0,0.1f });
        g_Sun = sun.GetHandle();
        sun.AddComponent<Light>()
	        .SetLightType(LightType::Point)
	        ->SetDirection({ 1,-1,-1 })
	        ->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        sun.SetName("Sun");

#if SAMPLE_IMPLEMENTATION

        Object& mercury = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        mercury.AddComponent<Renderer>(materialManager->GetMaterial("Mercury"), sphereMesh);
        mercury.GetComponentRef<Component::Transform>().SetPosition({ 1,0,0 }).SetScale(0.2f).SetRotation({ 0,0,0.1f });
        g_Mercury = mercury.GetHandle();
        mercury.SetName("Mercury");


        Object& venus = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        venus.AddComponent<Renderer>(materialManager->GetMaterial("Venus"), sphereMesh);
        venus.GetComponentRef<Component::Transform>().SetPosition({ 2,0,0 }).SetScale(0.25f).SetRotation({ 0,0,0.1f });
        g_Venus = venus.GetHandle();;
        venus.SetName("Venus");


        Object& earth = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        earth.AddComponent<Renderer>(materialManager->GetMaterial("Earth"), sphereMesh);
        earth.GetComponentRef<Component::Transform>().SetPosition({ 3,0,0 }).SetScale(0.3f).SetRotation({ 0,0,0.1f });
        g_Earth = earth.GetHandle();
        earth.SetName("Earth");


        Object& moon = g_MainScene.CreateChildObject(earth.GetHandle(), usingShader);
        Renderer& rendererComp2 = moon.AddComponent<Renderer>(nullptr, nullptr);
        rendererComp2.AssignMaterial(materialManager->GetMaterial("Moon")).AttachMesh(sphereMesh);
        Component::Transform& moonTrans = moon.GetComponentRef<Component::Transform>();
        moonTrans.SetPosition({ -1,0,0 }).SetRotation({ 0,0,0.1f });
        moonTrans.SetScale(0.3f);
        moon.AddComponent<Camera>(moonTrans, true, g_Graphics.get());
        g_Moon = moon.GetHandle();
        moon.SetName("Moon");



        Object& mars = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        Renderer& rendererCompMars = mars.AddComponent<Renderer>(nullptr, nullptr);
        rendererCompMars.AssignMaterial(materialManager->GetMaterial("Mars")).AttachMesh(sphereMesh);
        Component::Transform& marsTrans = mars.GetComponentRef<Component::Transform>();
        marsTrans.SetPosition({ 4, 0,0 }).SetRotation({ 0,0,0.1f });
        marsTrans.SetScale(0.3f);
        g_Mars = mars.GetHandle();
        mars.SetName("Mars");


        Object& jupiter = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        jupiter.AddComponent<Renderer>(materialManager->GetMaterial("Jupiter"), sphereMesh);
        jupiter.GetComponentRef<Component::Transform>().SetPosition({ 5,0,0 }).SetScale(0.8f).SetRotation({ 0,0,0.1f });
        g_Jupiter = jupiter.GetHandle();;
        jupiter.SetName("Jupiter");

        
        Object& saturn = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        saturn.AddComponent<Renderer>(materialManager->GetMaterial("Saturn"), sphereMesh);
        saturn.GetComponentRef<Component::Transform>().SetPosition({ 6,0,0 }).SetScale(0.6f).SetRotation({ 0,0,0.1f });
        g_Saturn = saturn.GetHandle();
        saturn.SetName("Saturn");

        
        Object& uranus = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        uranus.AddComponent<Renderer>(materialManager->GetMaterial("Uranus"), sphereMesh);
        uranus.GetComponentRef<Component::Transform>().SetPosition({ 7.5f,0,0 }).SetScale(0.5f).SetRotation({ 0,0,0.1f });
        g_Uranus = uranus.GetHandle();
        uranus.SetName("Uranus");

        
        Object& neptune = g_MainScene.CreateChildObject(sun.GetHandle(), usingShader);
        neptune.AddComponent<Renderer>(materialManager->GetMaterial("Neptune"), sphereMesh);
        neptune.GetComponentRef<Component::Transform>().SetPosition({ 9.5f,0,0 }).SetScale(0.55f).SetRotation({ 0,0,0.1f });
        g_Neptune = neptune.GetHandle();
        neptune.SetName("Neptune");


        Object& plane = g_MainScene.CreateObject(usingShader);
        plane.AddComponent<Renderer>(materialManager->GetMaterial("Plane"), planeMesh);
        plane.GetComponentRef<Component::Transform>().SetPosition({ 0,-10,-30 }).SetScale(0.05f).SetRotation({ 0,0,0 });
        plane.SetName("plane");
        
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        ///////////////////////////////////////////////////
        Object& camObj = g_MainScene.CreateObject(usingShader);
        Component::Transform& camtrans = camObj.GetComponentRef<Component::Transform>();
        camtrans.SetPosition({ 0,2,6 });
        camtrans.SetRotation({ -0.2f,0,0 });
        camtrans.SetScale(500.0f);
        camObj.AddComponent<Camera>(camtrans, true, g_Graphics.get());
        camObj.AddComponent<Skydome>(materialManager->GetMaterial("Skydome"), sphereReversedMesh);
        //camObj.AddComponent<Light>()
        //    .SetLightType(LightType::Directional)
        //    ->SetDirection({ 1,-1,-1 })
        //    ->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        g_Cam = camObj.GetHandle();
        camObj.SetName("Camera");
#endif // SAMPLE_IMPLEMENTATION

    }
    TwEditor::CreateComponentEditor("Object & Component", g_MainScene.GetEditorObjectRef(), g_Graphics);
    TwEditor::CreateResourceEditor("Resource Manager", g_Graphics->GetTextureManager()->GetAllTextures(), materialManager->GetAllMaterials(), g_Graphics);
    
    g_MainScene.StartScene();
}

//**************************************************************************
void Update(Application* /*application*/, float dt, void* /*userdata*/)
{
    g_MainScene.UpdateScene(dt);


#if SAMPLE_IMPLEMENTATION
    Object& sun = g_MainScene.GetObjectRef(g_Sun);
    Object& earth = g_MainScene.GetObjectRef(g_Earth);
    Object& mars = g_MainScene.GetObjectRef(g_Mars);
    Object& moon = g_MainScene.GetObjectRef(g_Moon);

    Object& mercury = g_MainScene.GetObjectRef(g_Mercury);
    Object& venus   = g_MainScene.GetObjectRef(g_Venus);
    Object& jupiter = g_MainScene.GetObjectRef(g_Jupiter);
    Object& saturn  = g_MainScene.GetObjectRef(g_Saturn);
    Object& uranus  = g_MainScene.GetObjectRef(g_Uranus);
    Object& neptune = g_MainScene.GetObjectRef(g_Neptune);
    Object& cam     = g_MainScene.GetObjectRef(g_Cam);


    Component::Transform& sunTrans      = sun.GetComponentRef<Component::Transform>();
    Component::Transform& earthTrans    = earth.GetComponentRef<Component::Transform>();
    Component::Transform& marsTrans     = mars.GetComponentRef<Component::Transform>();
    Component::Transform& moonTrans     = moon.GetComponentRef<Component::Transform>();
    Component::Transform& mercuryTrans  =    mercury.GetComponentRef<Component::Transform>();
    Component::Transform& venusTrans    =     venus  .GetComponentRef<Component::Transform>();
    Component::Transform& jupiterTrans  =    jupiter.GetComponentRef<Component::Transform>();
    Component::Transform& saturnTrans   =     saturn .GetComponentRef<Component::Transform>();
    Component::Transform& uranusTrans   =     uranus .GetComponentRef<Component::Transform>();
    Component::Transform& neptuneTrans  =    neptune.GetComponentRef<Component::Transform>();
    Component::Transform& camTrans      = cam.GetComponentRef<Component::Transform>();


    //sunTrans.Rotate(Vector3(0, dt * 0.11f, 0));
    //earthTrans.Rotate(Vector3(dt, dt, dt));
    //marsTrans.Rotate(Vector3(dt, dt, dt));
    earthTrans  .Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    marsTrans   .Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    moonTrans   .Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    mercuryTrans.Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    venusTrans  .Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    jupiterTrans.Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    saturnTrans .Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    uranusTrans .Rotate({dt*0.0f, dt*1.5f, dt*0.0f});
    neptuneTrans.Rotate({dt*0.0f, dt*1.5f, dt*0.0f});


    mercuryTrans.Orbit({ 0,1,0 }, dt * 4.15f);
    venusTrans.  Orbit({ 0,1,0 }, dt * 1.62f);
    earthTrans.  Orbit({ 0,1,0 }, dt * 1.00f);
    moonTrans.   Orbit({ 0,1,0 }, dt * 0.95f);
    marsTrans.   Orbit({ 0,1,0 }, dt * 0.53f);
    jupiterTrans.Orbit({ 0,1,0 }, dt * 0.8f);
    saturnTrans .Orbit({ 0,1,0 }, dt * 0.4f);
    uranusTrans .Orbit({ 0,1,0 }, dt * 0.1f);
    neptuneTrans.Orbit({ 0,1,0 }, dt * 0.5f);

#endif
    g_Graphics->RenderScene(&g_MainScene);
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
        application->GetWindowWidth(), application->GetWindowHeight());
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
}


//**************************************************************************
int main(int argc, char* argv[])
{
    Application* app = &Application::GetInstance();
    app->Initialize(argc, argv, "CS300 Course Framework", c_DefaultWindowWidth, c_DefaultWindowHeight);
    app->SetOnViewportChanged(OnViewportChanged);
    app->SetMouseWheelCallback(OnMouseWheel);
    app->SetMouseButtonDownCallback(OnMouseButtonDown);
    app->Run(Initialize, Loading, Cleanup);
    return 0;
}
