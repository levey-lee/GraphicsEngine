#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/Color.h"
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"

namespace Graphics
{
  Framebuffer::Framebuffer(u32 width, u32 height)
    : m_width(width), m_height(height), m_fbo(NULL), m_depthRbo(NULL),
    m_clearColor(Color::Black), m_colorTexture(NULL)
  {
  }

  void Framebuffer::Build()
  {
    // build texture
    u8 *data = new u8[m_width * m_height * 4];
    std::memset(data, 0xffffffff, sizeof(u8) * m_width * m_height * 4);
    m_colorTexture = std::make_shared<Texture>(
      data, m_width, m_height, Texture::Format::RGBA);
    m_colorTexture->Build();
    // build framebuffer, and renderbuffer
    glGenFramebuffers(1, &m_fbo);
    glGenRenderbuffers(1, &m_depthRbo);
    // fill in renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
      m_width, m_height);
    // link framebuffer
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      m_colorTexture->GetTextureHandle(), 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_RENDERBUFFER, m_depthRbo);
    // verify it worked
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Assert(status == GL_FRAMEBUFFER_COMPLETE, "Failed to create framebuffer.");
    glViewport(0, 0, m_width, m_height);
    glClearColor(0.f, 0.f, 0.f, 1.f); // default clear color
    glClearDepth(1.f); // default clear depth
    Unbind();
  }

  void Framebuffer::Bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    WarnIf(m_fbo == NULL, "Warning: Binding unbuilt framebuffer.");
  }

  void Framebuffer::Clear()
  {
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  void Framebuffer::Unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind screen framebuffer
  }

  void Framebuffer::Destroy()
  {
    Unbind();
    m_colorTexture->Destroy();
    glDeleteRenderbuffers(1, &m_depthRbo);
    glDeleteFramebuffers(1, &m_fbo);
    m_fbo = NULL;
    m_depthRbo = NULL;
    m_colorTexture = NULL; // deletes it
  }
}
