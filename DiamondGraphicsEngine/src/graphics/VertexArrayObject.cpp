#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/VertexArrayObject.h"
#include "graphics/Mesh.h"

namespace Graphics
{
    VertexArrayObject::VertexArrayObject(size_t vertexCount,
                                         size_t primitiveCount, size_t vertexSize, Topology topology/* = TRIANGLES*/)
        : m_vertexArrayHandle(NULL), m_ibo(topology, primitiveCount),
          m_vbo(vertexCount, vertexSize)
    {
    }

    VertexArrayObject::~VertexArrayObject()
    {
        // cleanup
        glDeleteVertexArrays(1, &m_vertexArrayHandle);
    }

    void VertexArrayObject::Build(Mesh* mesh)
    {// create a new vertex array object
        glGenVertexArrays(1, &m_vertexArrayHandle);
        Assert(m_vertexArrayHandle, "Failed to create vertex array.");
        CheckGL();

        // bind the VAO to prepare for binding the vertex input layout
        Bind();
        {
            // count the size of a vertex using the Vertex structure
            size_t const vertexSize = mesh->GetVertexSize();

            // build VBO
            m_vbo.Build();

            // Enable attributes and set their pointers; this initializes the vertex
            // input layout. When rendering with a VBO, OpenGL needs to be told how
            // to read the VBO's data buffer. It needs to be told how different
            // offsets in the buffer correspond to different input vertex attributes
            // to the vertex shader. This is done using glEnableVertexAttribArray and
            // glVertexAttribPointer. Typically, it's done in two steps during render
            // time. With a VAO, however, we do them both together during
            // initialization since the VAO will remember this layout and
            // automatically bind it for us when we bind the VAO.
            size_t offset = 0;
            size_t attributeCount = mesh->GetAttributeCount();
            for (size_t i = 0; i < attributeCount; ++i)
            {
                // Tells OpenGL to accept input vertices for a layout with this index;
                // think of GLSL code like: layout(location = 0) in vec3 vVertex; The
                // number portion of this code corresponds to 'i' in this for-loop.
                glEnableVertexAttribArray(static_cast<GLuint>(i));

                // Tells OpenGL where within EACH VERTEX inside the VBO this attribute
                // comes from. For a Vertex struct containing a vVertex (vec3) and a
                // vNormal (vec3), we first indicate location 0 (vVertex) has an
                // element count of 3, each element is of type GL_FLOAT, the total
                // vertex size is 24 bytes, and the offset to the vertex data within
                // the Vertex struct is 0 (casted to GLvoid *). During the second loop,
                // location 1 (vNormal) has an element count of 3, each element is of
                // type GL_FLOAT, the total vertex size is still 24 bytes, and the
                // offset to the normal within the Vertex structure is 12 (since it
                // starts after the last byte of vVertex). The GL_FALSE specifies
                // whether the input data should be normalized.
                glVertexAttribPointer(static_cast<GLuint>(i),
                    static_cast<GLint>(mesh->GetAttributeElementCounts()[i]), GL_FLOAT, GL_FALSE,
                    vertexSize, reinterpret_cast<GLvoid *>(offset));

                CheckGL();
                offset += mesh->GetAttributeElementSizes()[i]; // skip to the next attribute
            }

            // build IBO
            m_ibo.Build();
        }
        // finished
        Unbind();
    }

    void VertexArrayObject::BuildForSampleTriangle(Mesh* mesh)
    {
        m_vbo.AddVertex(Math::Vec3(0, 0.5f, -1));
        m_vbo.AddVertex(Math::Vec3(-0.5f, -0.5f, -1));
        m_vbo.AddVertex(Math::Vec3(0.5f, -0.5f, -1));
        m_ibo.AddTriangle(0,1,2);
        Build(mesh);
    }

    void VertexArrayObject::Bind()
    {
        // bind the vertex array object, automatically binding the VBO, IBO, and
        // setting up the vertex input layout for us
        Assert(m_vertexArrayHandle, "Cannot bind unbuilt vertex array.");
        glBindVertexArray(m_vertexArrayHandle);
    }

    void VertexArrayObject::Render()
    {
        // glDrawElements instructs OpenGL to use the current VBO and IBO to draw
        // geometry by going through the contents of the IBO and extracting 3
        // indexes (if GL_TRIANGLES is the mode) of type GL_UNSIGNED_INT (32-bit)
        // and using those to lookup the vertices inside the current VBO and
        // render that triangle, for n total indices. The NULL at the end is used
        // to specify the actual index array stored CPU-side; this comes from older
        // OpenGL behavior before IBOs existed. Specifying NULL tells OpenGL to use
        // the currently bound GL_ELEMENT_ARRAY_BUFFER instead of copying over
        // memory from the CPU-side each draw call.
        glDrawElements(m_ibo.GetTopology() == Topology::TRIANGLES
                           ? GL_TRIANGLES
                           : GL_LINES, m_ibo.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
    }

    void VertexArrayObject::Unbind()
    {
        // unbind the vertex array object and any contained objects
        glBindVertexArray(NULL);
    }
}
