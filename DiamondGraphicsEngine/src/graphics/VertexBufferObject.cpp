#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/VertexBufferObject.h"
#include "graphics/TriangleMesh.h"

namespace Graphics
{
  // Constructs a new VBO using all these parameters, including the actual data
  // buffer used to store vertices. Notice how the buffer size is based on the
  // size of the Vertex structure, as well as the number of vertices specified.
  VertexBufferObject::VertexBufferObject(size_t vertexCount, size_t vertexSize)
    : m_vertexCount(vertexCount), m_insertOffset(0),
    m_bufferSize(vertexSize * vertexCount),
    m_buffer(new char[m_bufferSize]), m_glHandle(0)
  {
  }

  VertexBufferObject::~VertexBufferObject()
  {
    // cleanup
      VertexBufferObject::Destroy();
    delete[] m_buffer;
  }
  
  size_t VertexBufferObject::GetBufferSize() const
  {
    return m_bufferSize;
  }

  void VertexBufferObject::Build()
  {
    Assert(!m_glHandle, "Error: trying to build already-built vertex buffer.");
    if (m_glHandle)
      return;

    // glGenBuffers creates a new OpenGL buffer object with no size. It actually
    // creates an array of objects, but we are specifying just needing one.
    glGenBuffers(1, &m_glHandle);
    Assert(m_glHandle, "Error: failed to create vertex buffer.");
    CheckGL();
    if (!m_glHandle)
      return;

    // glBufferData initializes the buffer object to be an ARRAY_BUFFER with the
    // specified size and initial data. This is why the buffer must be
    // initialized with all of its contained vertices before building the VBO.
    // GL_STATIC_DRAW hints to the driver that we are only going to initialize
    // this buffer, never modify it, and only use it to draw. It allows for a
    // potential speed boost while rendering by storing the VBO contents in
    // actual VRAM (not guaranteed).
    Bind();
    glBufferData(GL_ARRAY_BUFFER, m_bufferSize, m_buffer, GL_STATIC_DRAW);
    CheckGL();
  }

  void VertexBufferObject::Bind() const
  {
    // bind the buffer as a GL_ARRAY_BUFFER (can only have one of those bound
    // at a time)
    glBindBuffer(GL_ARRAY_BUFFER, m_glHandle);
    CheckGL();
  }

  void VertexBufferObject::Unbind() const
  {
    // unbinds the buffer by binding the special reserved value of 0
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void VertexBufferObject::Destroy()
  {
    if (!m_glHandle)
      return; // already destroyed

    // deletes an array of buffers, but we only have one to delete
    glDeleteBuffers(1, &m_glHandle);
  }
}
