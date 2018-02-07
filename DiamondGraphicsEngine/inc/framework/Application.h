#ifndef H_APPLICATION
#define H_APPLICATION
#include "framework/Singleton.h"
#include "core/Ray.h"

namespace Math {
    struct Vector2;
}

// using OpenGL. Due to limitations with FreeGLUT, only one Application
// instance may exist per process.
class Application
    : public Singleton<Application>
{
    friend struct ApplicationWrapper;
public:
    using InitClientCallBack = void(*)(Application* application, void* userData);
    using UpdateClientCallBack = void(*)(Application* application, float deltaTime, void* userData);
    using CleanUpClientCallBack = void(*)(Application* application, void* userData);
    using ViewportChangedCallBack = void(*)(Application* application);
    using OnKeyDownCallBack = void(*)(Application* application, unsigned char key, int x, int y);
    using OnKeyUpCallBack = void(*)(Application* application, unsigned char key, int x, int y);
    using OnSpecialKeyDown = void(*)(Application* application, int key, int x, int y);
    using OnSpecialKeyUp = void(*)(Application* application, int key, int x, int y);
    using OnMouseButtonDownCallBack = void(*)(Application* application, int button, int x, int y);
    using OnMouseButtonUpCallBack = void(*)(Application* application, int button, int x, int y);
    using OnMouseWheelCallBack = void(*)(Application* application, int wheel, int direction, int x, int y);
    using OnMouseDragCallBack = void(*)(Application* application, int x, int y);

    Application(){}
    ~Application() {}
    std::string const& GetTitle() const { return m_windowTitle; }
    unsigned GetWindowWidth() const { return m_windowWidth; }
    unsigned GetWindowHeight() const { return m_windowHeight; }

    // Creates a new GL 3.3 context using FreeGLUT and GLEW. Creates and opens
    // the window for the application. Verifies the context has been successfully
    // created.
    void Initialize(int argc, char* argv[], std::string const& title, unsigned width,
                    unsigned height);

    // Initializes callbacks for input for GLUT, initializes editor, and begins
    // the main process loop. The initCallback is called exactly once before the
    // update loop begins. The deinitCallback is called exactly once after the
    // update loop is no longer executing (such as when the user clicks the exit
    // button). The update loop is continuously called as long as the application
    // is not running. You can use 'Application::close' to exit the application
    // at any time. It properly handles calling the deinitCallback in that case,
    // so you may clean up OpenGL objects and any other memory used. This
    // function blocks until Close is called.
    void Run(InitClientCallBack initCallback, UpdateClientCallBack updateCallback,
             CleanUpClientCallBack cleanupCallback, void* initUserData = nullptr,
             void* updateUserData = nullptr, void* cleanupUserData = nullptr);


    static Ray GetRayFromScreenCoords(int x, int y, Math::Vector3 const& cameraPos,
        Math::Matrix4 const& cameraViewMat, Math::Matrix4 const& cameraProjectMat);

    // Safely closes the application, leading to the cleanupCallback passed to
    // Update to be called and, eventually, thread control to be restored to the
    // caller of Run.
    void Close();

    void SetUpdateFunc(UpdateClientCallBack func) { m_updateCallback = func; }
    void SetOnViewportChanged(ViewportChangedCallBack func) { m_viewportCallback = func; }
    void SetMouseWheelCallback(OnMouseWheelCallBack func) { m_mouseViewCallback = func; }
    void SetMouseButtonDownCallback(OnMouseButtonDownCallBack func) { m_mouseButtonDownCallBack = func; }
    void SetKeyDownCallback(OnKeyDownCallBack func) { m_keyDownCallBack = func; }
    void SetMouseButtonUpCallback(OnMouseButtonUpCallBack func) { m_mouseButtonUpCallBack = func; }
    void SetMouseDragCallback(OnMouseDragCallBack func) { m_mouseButtonDragCallBack = func; }
    //TODO: add more functions like above, if you wish to handle more events

    struct ApplicationWrapper;
private:

    std::string m_windowTitle;
    unsigned m_windowWidth = 0;
    unsigned m_windowHeight = 0;
    unsigned m_windowHandle = 0;
    void* m_updateCallbackData = nullptr;
    void* m_cleanupCallbackData = nullptr;
    UpdateClientCallBack m_updateCallback = nullptr;
    CleanUpClientCallBack m_cleanupCallback = nullptr;
    ViewportChangedCallBack m_viewportCallback = nullptr;
    OnMouseWheelCallBack m_mouseViewCallback = nullptr;
    OnMouseButtonDownCallBack m_mouseButtonDownCallBack = nullptr;
    OnKeyDownCallBack m_keyDownCallBack = nullptr;
    OnMouseButtonUpCallBack m_mouseButtonUpCallBack = nullptr;
    OnMouseDragCallBack m_mouseButtonDragCallBack = nullptr;






};


#endif
