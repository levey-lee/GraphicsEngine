#include "Precompiled.h"
#include "framework/Application.h"
#include "graphics/Framebuffer.h"
#include "graphics/FramebufferManager.h"

namespace
{
  // Kept as a file-scope global so that it can be returned as a const ref.
  static std::shared_ptr<Graphics::Framebuffer> const NullFramebuffer = NULL;
}

namespace Graphics
{
  FramebufferManager::FramebufferManager(Application *application)
    : m_application(application), m_screenClearColor(Color::Gray),
    m_framebuffers()
  {
  }

  FramebufferManager::~FramebufferManager()
  {
  }

  std::shared_ptr<Framebuffer> const &FramebufferManager::RegisterFramebuffer(
    FramebufferType type, u32 width, u32 height)
  {
    if (type == FramebufferType::SCREEN)
      return NullFramebuffer; // don't allow screen to be replaced

    // create new framebuffer given dimensions; save the framebuffer given the
    // specified type or, if one is already associated with that type, replace
    // it with the newly loaded framebuffer
    auto find = m_framebuffers.find(type);
    auto fb = std::make_shared<Framebuffer>(width, height);
    if (find != m_framebuffers.end())
    {
      find->second = fb; // replace framebuffer
      return find->second;    // and return it
    }
    else // new fb: insert(std::pair<FramebufferType, ...>(type, fb))
      return m_framebuffers.emplace(type, fb).first->second; // and return it
  }

  std::shared_ptr<Framebuffer> const &FramebufferManager::GetFramebuffer(
    FramebufferType key) const
  {
    // find a framebuffer given the specified type; if it doesn't exist, return
    // null instead
    auto find = m_framebuffers.find(key);
    return (find != m_framebuffers.end()) ? find->second : NullFramebuffer;
  }

  void FramebufferManager::ClearFramebuffers()
  {
    m_framebuffers.clear();
  }

  void FramebufferManager::Bind(FramebufferType type) const
  {
    if (type == FramebufferType::SCREEN)
    {
      // binding 0 framebuffer unbinds previous, thereby binding the screen
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(0, 0, m_application->GetWindowWidth(),
        m_application->GetWindowHeight());
    }
    else
    {
      auto framebuffer = GetFramebuffer(type);
      if (framebuffer)
        framebuffer->Bind();
    }
  }

  void FramebufferManager::SetClearColor(FramebufferType type,
    Color const &color)
  {
    if (type == FramebufferType::SCREEN)
      m_screenClearColor = color;
    else
    {
      auto framebuffer = GetFramebuffer(type);
      if (framebuffer)
        framebuffer->SetClearColor(color);
    }
  }

  void FramebufferManager::Clear(FramebufferType type) const
  {
    if (type == FramebufferType::SCREEN)
    {
      // reset clear color in case it changed
      glClearColor(m_screenClearColor.r, m_screenClearColor.g,
        m_screenClearColor.b, 1.f); // no alpha for screen
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    else
    {
      auto framebuffer = GetFramebuffer(type);
      if (framebuffer)
        framebuffer->Clear();
    }
  }
}
