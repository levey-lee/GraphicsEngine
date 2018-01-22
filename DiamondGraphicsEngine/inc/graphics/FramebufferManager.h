#ifndef H_FRAMEBUFFER_MANAGER
#define H_FRAMEBUFFER_MANAGER

#include "graphics/Color.h"

class Application;

namespace Graphics
{
  class Framebuffer;

  // This enum stores all the known types of framebuffers used by the
  // application. Add all needed framebuffers to this for assignment 4.
  enum class FramebufferType
  {
    // do NOT remove this; it's a reserved value used by the manager to know to
    // bind the screen framebuffer itself, which is a special OGL state
    SCREEN = 0,
    EXAMPLE = 1,

    // TODO(student): For Deferred Shader(Assignment 3), add more types here, such as POSITIVE_Z or NEGATIVE_X

    COUNT
  };

  class FramebufferManager
  {
  public:
    FramebufferManager(Application *application);
    ~FramebufferManager();

    // Creates a new framebuffer given dimensions and a key. Will return NULL
    // if you try to override the SCREEN framebuffer.
    std::shared_ptr<Framebuffer> const &RegisterFramebuffer(
      FramebufferType type, u32 width, u32 height);

    // Retreives a framebuffer based on its type. If the type is unknown, this
    // method returns NULL. This will return NULL for the SCREEN type, since
    // theb screen is not a framebuffer object.
    std::shared_ptr<Framebuffer> const &GetFramebuffer(
      FramebufferType key) const;

    // Destroys all shader programs stored within this manager. This is handy
    // to cleanup any shader resources being used by the application.
    void ClearFramebuffers();

    // Convenience method for binding the framebuffer of the specified type.
    // This automatically handles sizing the viewport and has convenience
    // functionality for binding the screen (same as unbinding).
    void Bind(FramebufferType type) const;

    // This is a convenience method since setting the clear color for a
    // framebuffer is different than the screen. This wraps it all nicely into
    // one method.
    void SetClearColor(FramebufferType type, Color const &color);

    // Convenience method for clearing a specific framebuffer. Must be bound.
    void Clear(FramebufferType type) const;

  private:

    // Disallow copying of this object.
    FramebufferManager(FramebufferManager const &) = delete;
    FramebufferManager &operator=(FramebufferManager const &) = delete;

    Application *m_application; // used to track screen's dimensions
    Color m_screenClearColor;
    std::unordered_map<FramebufferType, std::shared_ptr<Framebuffer> > m_framebuffers;
  };
}

#endif
