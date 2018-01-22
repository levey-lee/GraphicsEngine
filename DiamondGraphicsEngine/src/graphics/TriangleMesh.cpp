#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/TriangleMesh.h"
#include "math/Math.h"

namespace Graphics
{
    using namespace Math;


    TriangleMesh::TriangleFace::TriangleFace(u32 _a, u32 _b, u32 _c)
        : a(_a), b(_b), c(_c)
    {
    }


    TriangleMesh::TriangleMesh()
        : m_vertices(), m_triangles(), m_triangleNormals()
    {
    }


    TriangleMesh::~TriangleMesh()
    {
    }


    void TriangleMesh::AddVertex(f32 x, f32 y, f32 z)
    {
        // push_back(Vertex(Vector3(x, y, z)))
        m_vertices.emplace_back(Vector3(x, y, z));
    }


    void TriangleMesh::AddTriangle(u32 a, u32 b, u32 c)
    {
        // push_back(Triangle(a, b, c))
        m_triangles.emplace_back(a, b, c);
    }

    void TriangleMesh::Build()
    {
        //TODO(Assignment 1): Build VAO, VBO, IBO for the mesh so that OpenGL will know how
        //to render this geometry
        // Construct a new VAO using each of the triangles and vertices stored
        // within this TriangleMesh; this is done in a less efficient way possible.
        // A simple memcpy could actually be used to copy data from the mesh
        // directly into the IBO and VBO.
#if SAMPLE_IMPLEMENTATION
        VertexArrayObject* array = new VertexArrayObject(m_vertices.size(), m_triangles.size(), sizeof(Vertex), Topology::TRIANGLES);
        VertexBufferObject& vbo = array->GetVertexBufferObject();
        IndexBufferObject& ibo = array->GetIndexBufferObject();

        // add all of the vertices to the VBO
        for (size_t i = 0; i < m_vertices.size(); ++i)
            vbo.AddVertex(m_vertices[i]);

        // add all indices (per triangle) to the IBO
        for (size_t i = 0; i < m_triangles.size(); ++i)
        {
            TriangleFace& tri = m_triangles[i];
            ibo.AddTriangle(tri.a, tri.b, tri.c);
        }

        // upload the contents of the VBO and IBO to the GPU and build the VAO
        array->Build(this);

        // wrap the VAO into a shared_ptr and save it locally to be rendered
        m_vertexArrayObject = std::shared_ptr<VertexArrayObject>(array);
#else
        // Step 0: Remove these two lines
        m_vertexArrayObject = std::make_shared<VertexArrayObject>(3, 1, 32U, (Topology)3);
        m_vertexArrayObject->BuildForSampleTriangle(this);
        // Step 1: allcate a new VAO
        // Step 2: add all of the vertices to the VBO
        // Step 3: add all indices (per triangle) to the IBO
        // Step 4: upload the contents of the VBO and IBO to the GPU and build the VAO
        // Step 5: wrap the VAO into a shared_ptr and save it locally to be rendered        
#endif // SAMPLE_IMPLEMENTATION

        m_isBuilt = true;
    }

    void TriangleMesh::Preprocess()
    {
        // various useful steps for preparing this model for rendering; none of
        // these would be done for a game
        centerMesh();
        normalizeVertices();
        CalculateBoundingSphere();

        //TODO(Assignment 1): Calculate all the information a vertex needs
        //You should firstly calculate face normal and store them in m_triangleNormals
        //then you average adjacent face normals to get vertex normal then store it in 
        //each corresponding vertex.
#if SAMPLE_IMPLEMENTATION

        std::vector<std::vector<unsigned>> adjList;
        adjList.resize(this->GetVertexCount());

        this->m_triangleNormals.clear();
        for (int i = 0, j = this->GetPrimitiveCount(); i < j; ++i)
        {
            TriangleFace& tri = m_triangles[i];

            adjList[tri.a].push_back(i);
            adjList[tri.b].push_back(i);
            adjList[tri.c].push_back(i);

            Vector3 n = Cross(m_vertices[tri.b].position - m_vertices[tri.a].position,
                m_vertices[tri.c].position - m_vertices[tri.a].position);
            m_triangleNormals.push_back(n);
        }
        //todo add more such as tan and bitan
        for (unsigned i = 0, j = adjList.size(); i < j; ++i)
        {
            Math::Vector3 sum(0, 0, 0);
            Math::Vector3 tSum(0, 0, 0);
            Math::Vector3 btSum(0, 0, 0);
            for (unsigned v = 0; v < adjList[i].size(); ++v)
            {
                bool addVec = true;
                for (unsigned k = 0; k < v; ++k)
                {
                    if (m_triangleNormals[adjList[i][k]] == m_triangleNormals[adjList[i][v]])
                    {
                        //normal has been added before, don't add again
                        addVec = false;
                        k = v;
                    }
                }
                if (addVec)
                {
                    sum += m_triangleNormals[adjList[i][v]];
                    tSum += m_triangleNormals[adjList[i][v]];
                    btSum += m_triangleNormals[adjList[i][v]];
                }
            }
            m_vertices[i].normal = sum.Normalized();
            //m_vertices[i].tangent = tSum.Normalized();
            //m_vertices[i].bitangent = btSum.Normalized();
        }

#endif // 0

        for (auto& i : m_triangleNormals)
        {
            i.AttemptNormalize();
        }
    }


    size_t TriangleMesh::GetVertexCount()
    {
        return m_vertices.size();
    }


    size_t TriangleMesh::GetPrimitiveCount()
    {
        return m_triangles.size();
    }


    Math::Vector3 TriangleMesh::GetFaceCentroid(u32 tidx) const
    {
        // average all vertices of the triangle to find the centroid
        TriangleFace const& tri = m_triangles[tidx];
        Vector3 centroid = m_vertices[tri.a].position;
        centroid += m_vertices[tri.b].position;
        centroid += m_vertices[tri.c].position;
        return centroid * (1.f / 3.f);
    }


    TriangleMesh::Vertex const& TriangleMesh::GetVertex(u32 vidx) const
    {
        Assert(vidx < m_vertices.size(), "Error: vertex index out of"
            " bounds: %d", vidx);
        return m_vertices[vidx];
    }


    TriangleMesh::Vertex& TriangleMesh::GetVertex(u32 vidx)
    {
        Assert(vidx < m_vertices.size(), "Error: vertex index out of"
            " bounds: %d", vidx);
        return m_vertices[vidx];
    }


    TriangleMesh::TriangleFace const& TriangleMesh::GetTriangle(u32 tidx) const
    {
        Assert(tidx < m_triangles.size(), "Error: triangle index out of"
            " bounds: %d", tidx);
        return m_triangles[tidx];
    }


    u32 TriangleMesh::GetPolygonIndex(u32 tidx, u8 position) const
    {
        Assert(tidx < m_triangles.size(), "Error: triangle index out of"
            " bounds: %d", tidx);
        Assert(position < 3, "Error: vertex index within triangle out of bounds: %d"
            " (expected 0 <= vertex < 3)", position);
        return m_triangles[tidx].indices[position];
    }


    Vector3 TriangleMesh::GetPolygonNormal(u32 tidx) const
    {
        Assert(tidx < m_triangleNormals.size(), "Error: triangle normal index"
            " out of bounds: %d", tidx);
        return m_triangleNormals[tidx];
    }


    void TriangleMesh::Render()
    {
        Assert(m_isBuilt, "TriangleMesh with label \"%s\" is not built.", m_label.c_str());
        // if the VAO has been built for this mesh, bind and render it
        if (m_vertexArrayObject)
        {
            m_vertexArrayObject->Bind();
            m_vertexArrayObject->Render();
            m_vertexArrayObject->Unbind();
        }
    }

    void TriangleMesh::CalculateBoundingSphere()
    {
        m_boudingSphere.center = m_center;
        float farestDistSq = 0;
        for (auto& i : m_vertices)
        {
            float distSq = i.position.LengthSq();
            if (distSq > farestDistSq)
            {
                farestDistSq = distSq;
            }
        }
        m_boudingSphere.radius = Math::Sqrt(farestDistSq);
    }

    size_t TriangleMesh::GetVertexSize()
    {
        return sizeof Vertex;
    }

    std::vector<size_t> TriangleMesh::GetAttributeElementSizes()
    {
        std::vector<size_t> eleSizes = {
            sizeof Vertex::position,
            sizeof Vertex::normal,
            sizeof Vertex::uv,
        };

#ifdef _DEBUG
        size_t eleSize = std::accumulate(eleSizes.begin(), eleSizes.end(), 0);
        Assert(eleSize==sizeof Vertex,"Vertex size mismatch. Did you add new element in the vertex but forget to modify related functions?");
#endif // _DEBUG
        return eleSizes;
    }

    std::vector<size_t> TriangleMesh::GetAttributeElementCounts()
    {
        std::vector<size_t> eleCounts = { 3,3, 2 };
        return eleCounts;
    }

    size_t TriangleMesh::GetAttributeCount()
    {
        return GetAttributeElementCounts().size();
    }

    /* helper methods */


    void TriangleMesh::centerMesh()
    {
        // find the centroid of the entire mesh (average of all vertices, hoping for
        // no overflow) and translate all vertices by the negative of this centroid
        // to ensure all transformations are about the origin
        for (auto& vert : m_vertices)
            m_center += vert.position;
        m_center *= 1.f / static_cast<f32>(m_vertices.size());
        // translate by negative centroid to center model at (0, 0, 0)
        m_center = -m_center;
        for (auto& vert : m_vertices)
            vert.position += m_center;
    }


    void TriangleMesh::normalizeVertices()
    {
        // find the extent of this mesh and normalize all vertices by scaling them
        // by the inverse of the smallest value of the extent (that isn't zero)
        Vector3 minimum = m_vertices[0].position;
        Vector3 maximum = m_vertices[0].position;
        for (auto& vert : m_vertices)
        {
            f32 x = vert.position.x, y = vert.position.y, z = vert.position.z;
            minimum.x = std::min(minimum.x, x);
            minimum.y = std::min(minimum.y, y);
            minimum.z = std::min(minimum.z, z);
            maximum.x = std::max(maximum.x, x);
            maximum.y = std::max(maximum.y, y);
            maximum.z = std::max(maximum.z, z);
        }
        Vector3 extent = maximum - minimum;
        f32 minExtentLength = 0.f;
        bool xZero = IsZero(extent.x);
        bool yZero = IsZero(extent.y);
        bool zZero = IsZero(extent.z);
        if (xZero && yZero && zZero)
            return; // cannot normalize 1 point mesh
        else if (xZero) // 2 or less may be zero
        {
            if (yZero)
                minExtentLength = extent.z; // only non-zero
            else if (zZero)
                minExtentLength = extent.y;
            else // only x is zero
                minExtentLength = std::min(extent.y, extent.z);
        }
        else if (yZero)
        {
            if (xZero)
                minExtentLength = extent.z; // only non-zero
            else if (zZero)
                minExtentLength = extent.x;
            else // only x is zero
                minExtentLength = std::min(extent.x, extent.z);
        }
        else if (zZero)
        {
            if (xZero)
                minExtentLength = extent.y; // only non-zero
            else if (yZero)
                minExtentLength = extent.x;
            else // only x is zero
                minExtentLength = std::min(extent.x, extent.y);
        }
        else
            minExtentLength = std::min(std::min(extent.x, extent.y), extent.z);
        f32 scalar = 1.f / minExtentLength; // guaranteed to not be 1/0
        for (auto& vert : m_vertices)
            vert.position *= scalar;
    }

    TriangleMesh* TriangleMesh::CalcUvSpherical()
    {
		for (auto& i : m_vertices)
		{

			Vector3 vec = i.position.Normalized();
			float theta = std::atan2(-vec.z, vec.x);
			float u = (theta + Math::c_Pi) / Math::c_TwoPi;
			float phi = std::acos(vec.y);
			float v = (phi) / Math::c_Pi;
			i.uv.x = Clamp(u);
			i.uv.y = Clamp(v);
		}
        return this;
    }

    TriangleMesh* TriangleMesh::CalcUvBox()
    {
        for (auto& i : m_vertices)
        {
            Vector3 position = i.position.Normalized();

            // find largest standard basis bias
            Vector3 mag = Math::Abs(position);
            Vector3 biasUVs = Vector3(0.5) + 0.5 * position;
            if (mag.x > mag.y && mag.x > mag.z)
            {
                // facing pos or neg x axis; use corrected y/z for UV
                i.uv.x = biasUVs.y;
                i.uv.y = biasUVs.z;
            }
            else if (mag.y > mag.z)
            {
                // facing pos or neg y axis; use corrected x/z for UV
                i.uv.x = biasUVs.x;
                i.uv.y = biasUVs.z;
            }
            else // z is the largest
            {
                // facing pos or neg z axis; use corrected x/y for UV
                i.uv.x = biasUVs.x;
                i.uv.y = biasUVs.y;
            }
        }
        return this;
    }
}
