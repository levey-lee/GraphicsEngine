#ifndef H_TRIANGLE_MESH
#define H_TRIANGLE_MESH

#include "framework/Utilities.h"
#include "math/Vector3.h"
#include "graphics/Mesh.h"
#include "graphics/MeshManager.h"

namespace Graphics
{
	/*******************************************************************
     * @brief 
     * This class represents the data structure for storing geometry data in a
     * convenient way to use directly with OpenGL. This data structure is designed
     * based on the Wavefront OBJ file format, but it is similar to a typical mesh
     * structure in a graphics engine. It should look similar to any mesh
     * structures you may have created for CS200/CS250 (especially CS250). You
     * will likely not need to change this interface much, but you may need to
     * change TriangleMesh::Build in future assignments. Most of the changes you
     * will be doing to meshes will be the Vertex itself. See Mesh and Mesh::Vertex for more
     * information on how vertices work. TriangleMeshes are intended to be easy to
     * assemble, convert to a VertexArrayObject, and render. The sample main file
     * provided with this framework demonstrates how to build a triangle mesh in
     * memory and render it.
     * @remark To have a better understanding, you should take a look at
     * the base class - Mesh
     ******************************************************************/
    class TriangleMesh
        : public Mesh
    {
        friend class MeshManager::TriangleMeshHandler;
    public:
        /*******************************************************
         * @brief vertex object used in OpenGL, data in the struct
         * will be passed to vertex shader as per vertex data
         * @remark WARNINGS: Whenever you add a thing to the struct below, you should also add
         * its data in GetAttributeElementSizes() and GetAttributeElementCounts() etc.
         * Please check the documentation in the mesh class for more detail.
         *******************************************************/
        struct Vertex
        {
            Math::Vector3 position = {0,0,0};       /* layout(location = 0) in vec3 vVertex;        */
            Math::Vector3 normal = {0,0,0};         /* layout(location = 1) in vec3 vNormal;        */
            Math::Vector2 uv = {0,0};               /* layout(location = 2) in vec3 vUv;            */
            Math::Vector3 tangent = {0,0, 0};       /* layout(location = 3) in vec3 vTangent;       */
            Math::Vector3 bitangent = {0,0, 0};     /* layout(location = 4) in vec3 vBitangent;     */

            
            Vertex() = default;
            Vertex(Math::Vector3 const& pos) :position(pos) {}
        };
		
        /*******************************************************
         * @brief
         * A typical triangle structure. A triangle is made up of 3 vertices,
         * therefore, 3 indices (just like the OBJ format). Bear in mind these
         * indices start at 0, instead of 1 like the format. They are direct indices
         * within the m_vertices vector stored inside the data structure. The union
         * allows the indices of a triangle to be accessed in two different ways
         * for convenience.
		*******************************************************/
        struct TriangleFace
        {
			/*******************************************************
			* @brief
			* You may wonder what a union is, in short, it's like
			* a struct, but all the members in the union share the same memory.
			*******************************************************/
            union
            {
                struct
                {
                    u32 a, b, c;
                };

                u32 indices[3]{};
            };

            TriangleFace(u32 _a, u32 _b, u32 _c);
        };

        TriangleMesh() = default;
        ~TriangleMesh() = default;

	    /*******************************************************************
         * @brief Adds a vertex to the m_vertices array. This could be used by a mesh loader
         * or a class creating a new mesh from scratch, such as a box or sphere.
         * @param x Position in x axis in object space.
         * @param y Position in y axis in object space.
         * @param z Position in z axis in object space.
         ******************************************************************/
        void AddVertex(f32 x, f32 y, f32 z);

	    /*******************************************************************
         * @brief 
         * Adds a triangle to the m_triangles array. This could be used by a mesh
         * loader or a class creating a new mesh from scratch, such as a box or
         * sphere.
         * @param a The first vertex index of this face.
         * @param b The second vertex index of this face.
         * @param c The third vertex index of this face.
         * @remark The order of a, b, c must be counter-clockwise.
         ******************************************************************/
        void AddTriangle(u32 a, u32 b, u32 c);

	    /*******************************************************************
         * @brief 
         * This performs preprocessing on the triangle mesh, such as computing
         * normals per face and vertex; centering all vertices about the origin;
         * and normalizing vertices to live within a range of [-0.5, 0.5]. This
         * should be called by a class loading the mesh or a class creating a new
         * mesh from memory.
         * @param defaultUvType Default uv type on the mesh.
         ******************************************************************/
        void Preprocess(DefaultUvType defaultUvType = DefaultUvType::None) override;

	    /*******************************************************************
         * @brief Retrieves the number of vertices stored within the mesh.
         * @return The total amount of vetices stored in this mesh.
         ******************************************************************/
        size_t GetVertexCount() override;

	    /*******************************************************************
         * @brief Retrieves the number of triangles stored within the mesh.
		 * @return How many faces in the mesh.
         ******************************************************************/
        size_t GetPrimitiveCount() override;

	    /*******************************************************************
         * @brief 
         * Computes the centroid of a particular triangle, given its index within
         * the array of triangles. The centroid is merely the average point of all
         * three points of the triangle. Since a triangle is always convex, this is
         * guaranteed to be within the triangle.
         * @param tidx The index of a face in this mesh.
         * @return The center of the face.
         ******************************************************************/
        Math::Vector3 GetFaceCentroid(u32 tidx) const override;

	    /*******************************************************************
         * @brief Gets a vertex, given an index.
         * @param vidx Index of a vertex
         * @return Vertex data
         ******************************************************************/
        Vertex const& GetVertex(u32 vidx) const;
		/*******************************************************************
		* @brief Gets a vertex, given an index.
		* @param vidx Index of a vertex
		* @return Vertex data
		******************************************************************/
        Vertex& GetVertex(u32 vidx);

	    /*******************************************************************
         * @brief  Gets a triangle, given an index.
         * @param tidx The index of a face
         * @return  A particular face of this mesh.
         ******************************************************************/
        TriangleFace const& GetTriangle(u32 tidx) const;

	    /*******************************************************************
         * @brief 
         * Gets the index of a relative vertex to a triangle. For example, calling
         * this method as such: GetPolygonIndex(2, 1) retrieves the second vertex
         * index of triangle 2 (the third triangle of the mesh; the second vertex
         * is value 'b' stored in Triangle above).
         * @param tidx The index of the triangle face
         * @param position The index of a vertex of the triangle, should be < 3
         * @return The index of a vertex of a triangle.
         ******************************************************************/
        u32 GetPolygonIndex(u32 tidx, u8 position) const override;

	    /*******************************************************************
         * @brief Gets the normal of a polygon, given its index. The normals of a polygon
         * are not guaranteed to exist until TriangleMesh::Preprocess() is caled.
         * @param tidx The index of a triangle face.
         * @return Face normal as vector3.
         ******************************************************************/
        Math::Vector3 GetPolygonNormal(u32 tidx) const override;

	    /*******************************************************************
         * @brief 
         * Builds a new VAO containing the information within this TriangleMesh and
         * using the specified ShaderProgram. See ShaderProgram.h and
         * VertexArrayObject.h (specifically VertexArrayObject::Build) for more
         * information. If the VAO of this mesh has already been constructed, this
         * replaces the old one with a new one, allowing the mesh to be changed
         * during runtime. This is not an efficient process to change it every
         * frame, but it would work.
         ******************************************************************/
        void Build() override;

	    /*******************************************************************
         * @brief 
         * Renders the VAO associated with this mesh, if it has been built using
         * the TriangleMesh::Build method. See VertexArrayObject::Render for more
         * information on how rendering meshes works.
         ******************************************************************/
        void Render() override;

        void CalculateBoundingSphere() override;

        size_t GetVertexSize() override;
        std::vector<size_t> GetAttributeElementSizes()override;
        std::vector<size_t> GetAttributeElementCounts()override;
        size_t GetAttributeCount()override;

		///////////////////////////////////////////////////////////////////////
		//		Helper functions to generate UVs as per vertex data
		///////////////////////////////////////////////////////////////////////
        TriangleMesh* CalcUvSpherical();
        TriangleMesh* CalcUvBox();
        TriangleMesh* CalcTanBitan();

    private:
	    /*******************************************************************
         * @brief 
         * Centers the mesh's vertices about the origin. This method is adaptive as
         * more vertices are added. Called by Preprocess().
         ******************************************************************/
        void centerMesh();

        // Normalizes the vertices within the extents [-0.5, 0.5]. This method is
        // adaptive as more vertices are added. Called by Preprocess().

		/*******************************************************************
		* @brief
		* Normalizes the vertices within the extents [-0.5, 0.5]. This method is
		* adaptive as more vertices are added. Called by Preprocess().
		******************************************************************/
        void normalizeVertices();

        Math::Vector3 m_center = { 0,0,0 };
        std::vector<Vertex> m_vertices;
        std::vector<TriangleFace> m_triangles;
        std::vector<Math::Vector3> m_triangleNormals;
        std::vector<Math::Vector3> m_tangent;
        std::vector<Math::Vector3> m_bitangent;

    };
}

#endif
