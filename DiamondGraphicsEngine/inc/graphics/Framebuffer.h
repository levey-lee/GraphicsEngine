#ifndef H_FRAMEBUFFER
#define H_FRAMEBUFFER

#include "graphics/Color.h"
#include "framework/Utilities.h"

namespace Graphics
{
  class ShaderProgram;
  class Texture;

  class Framebuffer
  {
  public:
    Framebuffer(u32 width, u32 height);

    u32 GetWidth() const { return m_width; }
    u32 GetHeight() const { return m_height; }

    u32 GetHandle() const { return m_fbo; }

    void SetClearColor(Color const &color) { m_clearColor = color; }
    Color const &GetClearColor() const { return m_clearColor; }

    void Build();
    void Bind(); // bind for rendering
    void Clear();
    std::shared_ptr<Texture> const &GetColorTexture() const {return m_colorTexture;}
    void Unbind();
    void Destroy();

  private:
    u32 m_width, m_height;
    u32 m_fbo, m_depthRbo;
    Color m_clearColor;
    std::shared_ptr<Texture> m_colorTexture;
  };
}

#endif
