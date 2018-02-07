#include "Precompiled.h"
#include "framework/Application.h"
#include "framework/Debug.h"
#include "math/Matrix4.h"

// These callbacks are wrapped in this struct so they may have private scope
// access to an instance of Application.
struct Application::ApplicationWrapper
{
    static void OnApplicationStartup(int);
    static void OnInitialize();
    static void OnClose();
    static void OnIdle();
    static void OnDraw();
    static void OnWindowResize(int width, int height);
    static void OnKeyDown(unsigned char key, int x, int y);
    static void OnKeyUp(unsigned char key, int x, int y);
    static void OnMouseButton(int button, int state, int x, int y);
    static void OnMouseButtonDown(int button, int x, int y);
    static void OnMouseButtonUp(int button, int x, int y);
    static void OnMouseWheel(int wheel, int direction, int x, int y);
    static void OnMouseDrag(int x, int y);
    static void OnMousePassiveMotion(int x, int y);
    static void OnSpecialKeyDown(int, int, int);
    static void OnSpecialKeyUp(int, int, int);
};


void Application::Initialize(int argc, char* argv[], std::string const& title, unsigned width,
                             unsigned height)
{
    m_windowTitle = title;
    m_windowWidth = width;
    m_windowHeight = height;


    // initialize GLUT & OpenGL 3.3-specific stuff
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    //glutInitContextVersion(4, 3); 
    glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitContextProfile(GLUT_FORWARD_COMPATIBLE);
    glutInitWindowSize(m_windowWidth, m_windowHeight);

    // safely close GLUT: http://stackoverflow.com/questions/5033832
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    // center window
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition((screenWidth / 2) - (m_windowWidth / 2),
                           (screenHeight / 2) - (m_windowHeight / 2));

    // open the window
    m_windowHandle = glutCreateWindow(m_windowTitle.c_str());

    // link OpenGL using GLEW
    glewExperimental = GL_TRUE;
    CheckGlew(glewInit());
    Assert(GLEW_VERSION_4_3, "OpenGL 4.3 not supported.");
}

void Application::Run(InitClientCallBack initCallback,
                      UpdateClientCallBack updateCallback, CleanUpClientCallBack cleanupCallback,
                      void* initUserData/* = NULL*/, void* updateUserData/* = NULL*/,
                      void* cleanupUserData/* = NULL*/)
{
    // initialize GLUT callbacks; each of these functions will be called, in turn,
    // based on OS events fired for each action; see for more information:
    // http://freeglut.sourceforge.net/docs/api.php#WindowCallback

	glutDisplayFunc(ApplicationWrapper::OnDraw);
	glutReshapeFunc(ApplicationWrapper::OnWindowResize);
	glutEntryFunc(ApplicationWrapper::OnApplicationStartup);
	glutSpecialFunc(ApplicationWrapper::OnSpecialKeyDown);
	glutSpecialUpFunc(ApplicationWrapper::OnSpecialKeyUp);
	glutKeyboardFunc(ApplicationWrapper::OnKeyDown);
	glutKeyboardUpFunc(ApplicationWrapper::OnKeyUp);
	glutMouseFunc(ApplicationWrapper::OnMouseButton);
	glutMouseWheelFunc(ApplicationWrapper::OnMouseWheel);
	glutMotionFunc(ApplicationWrapper::OnMouseDrag);
	glutPassiveMotionFunc(ApplicationWrapper::OnMousePassiveMotion);
	glutIdleFunc(ApplicationWrapper::OnIdle);
    glutCloseFunc(ApplicationWrapper::OnClose);
	TwGLUTModifiersFunc(glutGetModifiers);

    // initialize application
    m_updateCallback = updateCallback;
    m_updateCallbackData = updateUserData;
    m_cleanupCallback = cleanupCallback;
    m_cleanupCallbackData = cleanupUserData;
    ApplicationWrapper::OnInitialize();
    if (initCallback)
        initCallback(this, initUserData);

    // begin executing using GLUT
    glutMainLoop();

    // finished; return execution to the caller
}

void Application::Close()
{
    // cleanup the application and ImGui
    ApplicationWrapper::OnClose();
    glutLeaveMainLoop();
}

Ray Application::GetRayFromScreenCoords(int x, int y, Math::Vector3 const& cameraPos,
    Math::Matrix4 const& cameraViewMat, Math::Matrix4 const& cameraProjectMat)
{
    GLint viewport[4];
    GLdouble modelview[16] = { 0 };
    GLdouble projection[16] = { 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);

    //opengl uses column major matrix so they needs to be transposed.
    Math::Matrix4 viewMat = cameraViewMat.Transposed();
    Math::Matrix4 projMat = cameraProjectMat.Transposed();
    
    for (size_t i = 0; i < 16; i++)
    {
        modelview[i] = viewMat.array[i];
        projection[i] = projMat.array[i];
    }

    Math::Vector2 screenPos;
    screenPos.x = static_cast<float>(x);
    screenPos.y = static_cast<float>(viewport[3]) - static_cast<float>(y);

    GLfloat z_cursor;
    glReadPixels(static_cast<GLint>(screenPos.x), static_cast<GLint>(screenPos.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z_cursor);
    // obtain the world coordinates
    double rayX, rayY, rayZ;
    gluUnProject(screenPos.x, screenPos.y, z_cursor, modelview, projection, viewport, &rayX, &rayY, &rayZ);
    
    Ray ray;
    ray.SetStartPosition(cameraPos);
    Math::Vector3 direction = Math::Vector3(static_cast<float>(rayX), static_cast<float>(rayY), static_cast<float>(rayZ)) - ray.GetStartPosition();
    direction.AttemptNormalize();
    ray.SetRayDirection(direction);
    return ray;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////  Application Interface  ///////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Application::ApplicationWrapper::OnWindowResize(int width, int height)
{
    // framebuffer manager is to update the viewport now
    // update the dimensions as far as the application knows; this allows the
    // projection matrix being used to project to the correct dimensions
    Application* app = &GetInstance();
    app->m_windowWidth = width;
    app->m_windowHeight = height;
    if (app->m_viewportCallback)
    {
        app->m_viewportCallback(app);
    }

	TwWindowSize(GetInstance().m_windowWidth, GetInstance().m_windowHeight);
}
void Application::ApplicationWrapper::OnInitialize()
{
    std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << std::endl << std::endl;
}

void Application::ApplicationWrapper::OnDraw()
{
    static int elapsedTimeLastFrame = 0;
    int elapsedTime = glutGet(GLUT_ELAPSED_TIME);
    float dt = static_cast<float>(elapsedTime - elapsedTimeLastFrame);
    elapsedTimeLastFrame = elapsedTime;
    dt /= 1000.0f;
#ifdef _DEBUG
    static int titleUpdateCounter;
    if (titleUpdateCounter == 20)
    {
        glutSetWindowTitle((GetInstance().m_windowTitle + "  - fps[" + std::to_string(1.0f / dt) + "]").c_str());
        titleUpdateCounter = 0;
    }
    titleUpdateCounter++;
#endif // _DEBUG




    // allow client code to draw stuff with OpenGL
    GetInstance().m_updateCallback(&GetInstance(), static_cast<float>(dt),
                                   GetInstance().m_updateCallbackData);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0), in case of user error;
    glViewport(0, 0, GetInstance().m_windowWidth, GetInstance().m_windowHeight);


    // finish rendering; sync with graphics card and allow it to catch up
    glutSwapBuffers();

}

void Application::ApplicationWrapper::OnClose()
{   
    // call cleanup callback before terminating
    if (GetInstance().m_cleanupCallback)
        GetInstance().m_cleanupCallback(&GetInstance(), GetInstance().m_cleanupCallbackData);
}


void Application::ApplicationWrapper::OnMousePassiveMotion(int x, int y)
{
    TwEventMouseMotionGLUT(x, y);
}

void Application::ApplicationWrapper::OnSpecialKeyDown(int key, int x, int y)
{
	TwEventSpecialGLUT(key, x, y);
}

void Application::ApplicationWrapper::OnSpecialKeyUp(int, int, int)
{
}

void Application::ApplicationWrapper::OnApplicationStartup(int)
{
}


void Application::ApplicationWrapper::OnKeyDown(unsigned char key, int x, int y)
{
	TwEventKeyboardGLUT(key, x, y);
    if (key == 0x1B)//escape key
        GetInstance().Close();
}

void Application::ApplicationWrapper::OnKeyUp(unsigned char /*key*/, int, int)
{
}

void Application::ApplicationWrapper::OnMouseButton(int button, int state, int x, int y)
{
    if (TwEventMouseButtonGLUT(button, state, x, y) == 0)
    {
        if (state == 0)//button down
        {
            OnMouseButtonDown(button, x, y);
        }
        else if (state == 1)//button up
        {
            OnMouseButtonUp(button, x, y);
        }
    }
}
void Application::ApplicationWrapper::OnMouseButtonDown(int button, int x, int y)
{
    if (GetInstance().m_mouseButtonDownCallBack)
    {
        GetInstance().m_mouseButtonDownCallBack(&GetInstance(), button, x, y);
    }
}
void Application::ApplicationWrapper::OnMouseButtonUp(int /*button*/, int , int )
{
}

void Application::ApplicationWrapper::OnMouseWheel(int wheel, int direction, int x, int y)
{
    static int s_WheelPos = 0;
    s_WheelPos += direction;
    if (TwMouseWheel(s_WheelPos) == 0)
    {
        Application* app = &GetInstance();
        if (app->m_mouseViewCallback)
        {
            app->m_mouseViewCallback(app, wheel, direction, x, y);
        }
    }
}

void Application::ApplicationWrapper::OnMouseDrag(int x, int y)
{
	TwEventMouseMotionGLUT(x,y);
}

void Application::ApplicationWrapper::OnIdle()
{
    // continue to display as fast as possible
    glutPostRedisplay();
}
