#include "Precompiled.h"
#include "graphics/Mesh.h"

namespace Graphics
{
    void Mesh::Render()
    {
        Assert(m_isBuilt,"Mesh with label \"%s\" is not built.", m_label.c_str());
        // if the VAO has been built for this mesh, bind and render it
        if (m_vertexArrayObject)
        {
            m_vertexArrayObject->Bind();
            m_vertexArrayObject->Render();
            m_vertexArrayObject->Unbind();
        }
    }

    void Mesh::Reflect(TwBar* editor, std::string const& groupName, GraphicsEngine* )
    {
        std::string defStr = "group='" + groupName + "'";
        TwAddVarRO(editor, nullptr, TW_TYPE_BOOLCPP, &m_isBuilt, (defStr + " label='Is Built'").c_str());
    }

    size_t Mesh::GetVertexSize()
    {
        return sizeof Vertex;
    }

    std::vector<size_t> Mesh::GetAttributeElementSizes()
    {
        std::vector<size_t> eleSizes = {
            sizeof Vertex::position,
            sizeof Vertex::normal
        };
        return eleSizes;
    }

    std::vector<size_t> Mesh::GetAttributeElementCounts()
    {
        std::vector<size_t> eleCounts = { 3,3 };
        return eleCounts;
    }

    size_t Mesh::GetAttributeCount()
    {
        return GetAttributeElementCounts().size();
    }
}
