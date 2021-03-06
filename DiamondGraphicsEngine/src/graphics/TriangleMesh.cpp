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

        m_isBuilt = true;
    }

    void TriangleMesh::Preprocess(DefaultUvType defaultUvType)
    {
        // various useful steps for preparing this model for rendering; none of
        // these would be done for a game
        centerMesh();
        normalizeVertices();
        CalculateBoundingSphere();

        std::vector<std::vector<unsigned> > adjList;
        adjList.resize(this->GetVertexCount());

        m_triangleNormals.clear();
        for (int i = 0, j = m_triangles.size(); i < j; ++i)
        {
            TriangleFace& tri = m_triangles[i];

            adjList[tri.a].push_back(i);
            adjList[tri.b].push_back(i);
            adjList[tri.c].push_back(i);

            Vector3 n = Cross(m_vertices[tri.b].position - m_vertices[tri.a].position,
                m_vertices[tri.c].position - m_vertices[tri.a].position);
            m_triangleNormals.push_back(n);
        }

        for (unsigned i = 0, j = adjList.size(); i < j; ++i)
        {
            Math::Vector3 sum(0, 0, 0);

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
                }
            }
            m_vertices[i].normal = sum.Normalized();
        }

        for (auto& i : m_triangleNormals)
        {
            i.AttemptNormalize();
        }
        //calculate UV and tangents
        switch (defaultUvType)
        {
        case DefaultUvType::None:
            break;
        case Graphics::DefaultUvType::Box:
            CalcUvBox();
            break;
        case Graphics::DefaultUvType::Spherical:
        default:
            CalcUvSpherical();
            break;
        }
        CalcTanBitan();
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
            sizeof Vertex::tangent,
            sizeof Vertex::bitangent
        };

#ifdef _DEBUG
        size_t eleSize = std::accumulate(eleSizes.begin(), eleSizes.end(), 0);
        Assert(eleSize==sizeof Vertex,"Vertex size mismatch. Did you add new element in the vertex but forget to modify related functions?");
#endif // _DEBUG
        return eleSizes;
    }

    std::vector<size_t> TriangleMesh::GetAttributeElementCounts()
    {
        std::vector<size_t> eleCounts = { 3,3, 2, 3, 3 };
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

    TriangleMesh* TriangleMesh::CalcTanBitan()
    {
        //calculate face tangent and bitangent
        for (auto& i : m_triangles)
        {
            Vertex & v0 = m_vertices[i.a];
            Vertex & v1 = m_vertices[i.b];
            Vertex & v2 = m_vertices[i.c];

            Vector3 & p0 = v0.position;
            Vector3 & p1 = v1.position;
            Vector3 & p2 = v2.position;

            Vector2 & uv0 = v0.uv;
            Vector2 & uv1 = v1.uv;
            Vector2 & uv2 = v2.uv;

            Vector3 deltaPos1 = p1 - p0;
            Vector3 deltaPos2 = p2 - p0;

            Vector2 deltaUV1 = uv1 - uv0;
            Vector2 deltaUV2 = uv2 - uv0;

            float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
            Vector3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
            Vector3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

            m_tangent.push_back(tangent);
            m_bitangent.push_back(bitangent);
        }

        std::vector<std::pair<size_t, size_t> >tans;
        std::vector<std::pair<size_t, size_t> >bitans;
        for (size_t i = 0; i < m_vertices.size(); ++i)
        {
            tans.emplace_back(i, 0);
            bitans.emplace_back(i, 0);
        }

        //add face tangent and bitangent to vertices
        int index = 0;
        for (auto& i : m_triangles)
        {
            Vertex & v0 = m_vertices[i.a];
            Vertex & v1 = m_vertices[i.b];
            Vertex & v2 = m_vertices[i.c];
            
            ++tans[i.a].second;
            ++tans[i.b].second;
            ++tans[i.c].second;
            v0.tangent += m_tangent[index];
            v1.tangent += m_tangent[index];
            v2.tangent += m_tangent[index];

            ++bitans[i.a].second;
            ++bitans[i.b].second;
            ++bitans[i.c].second;
            v0.bitangent += m_bitangent[index];
            v1.bitangent += m_bitangent[index];
            v2.bitangent += m_bitangent[index];
            
            ++index;
        }

        //average vertex tangent and bitangent
        for (size_t i = 0; i < m_vertices.size();++i)
        {
           m_vertices[ tans[i].first ].tangent /= static_cast<float>(tans[i].second);
           m_vertices[bitans[i].first].bitangent /= static_cast<float>(bitans[i].second);
        }
        return this;
    }
}
