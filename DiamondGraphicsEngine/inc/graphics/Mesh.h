#pragma once
#include "framework/Utilities.h"
#include "math/Vector3.h"
#include "graphics/VertexArrayObject.h"
#include "core/BoundingSphere.h"
#include "core/HierarchicalObjectHandler.h"

namespace Graphics
{

	/*******************************************************************
     * @brief A generic mesh class for render engine. Common derived mesh
     *  types are triangle mesh, line mesh, quad polygon. You can also make
     *  your special mesh class.
     * @remark Any class derived from mesh needs to define struct Vertex
     * in its class as a public member struct if the shader layout changes.
     * Please also check the documentation of Vertex struct.
     ******************************************************************/
    class Mesh
    {
        /*******************************************************************
        * @brief Shader vertex data struct used for vertex buffer, index buffer
        * etc. Every mesh class needs to have this defined if the mesh contains
        * per vertex data other than position and normal.
        * @remark: Whenever you change or define this struct in the base or
        * derived class, you MUST also implement the four virtual functions.
        * They are GetVertexSize(), GetAttributeElementSizes()
        * GetAttributeElementCounts(), GetAttributeCount()
        ******************************************************************/
        struct Vertex
        {
            /* layout(location = 0) in vec3 vVertex; */
            Math::Vector3 position = { 0,0,0 };

            /* layout(location = 1) in vec3 vNormal; */
            Math::Vector3 normal = { 0,0,0 };

            Vertex() = default;
            Vertex(Math::Vector3 const& position);
        };
    public:
        Mesh()=default;
        virtual ~Mesh() = 0 {}

	    /*******************************************************************
		 * @brief Get the size of one shader vertex
		 * @return This should always return sizeof(Vertex).
		 * @remark You MUST implement this function if your Vertex struct
		 * is different than Vertex in base class.
		 ******************************************************************/
		virtual size_t GetVertexSize();
	    /*******************************************************************
		 * @brief 
		 * @return This function returns a vector of size_t, each element in 
		 * the vector represents the size of its corresponding element in the 
		 * Vertex struct. i.e. vector<>[0] is the size of the first element
		 * in the struct.
		 * @remark You MUST implement this function if your Vertex struct
		 * is different than Vertex in base class.
		 ******************************************************************/
		virtual std::vector<size_t> GetAttributeElementSizes();
	    /*******************************************************************
		 * @brief 
		 * @return This function returns a vector of size_t, each element in
		 * the vector represents total element count of its corresponding 
		 * element in the Vertex struct. e.g. the count of a Vector3 is 3,
		 * the count of a Vector2 is 2, the count of a matrix4 of 16.
		 * @remark You MUST implement this function if your Vertex struct
		 * is different than Vertex in base class.
		 ******************************************************************/
		virtual std::vector<size_t> GetAttributeElementCounts();
		virtual size_t GetAttributeCount();
	    /*******************************************************************
         * @brief This function modify the mesh before build. It is an option
         * for inheritance but useful. Usually you want to calculate normals,
         * UVs, set mesh etc in this function.
         ******************************************************************/
        virtual void Preprocess() {}
        /*******************************************************
         * @brief This functions gets how many vertex in this mesh.
         * @return Return the total amount of vertex.
         *******************************************************/
        virtual size_t GetVertexCount() = 0;
        /*******************************************************
         * @brief This functions gets how many elements in this mesh.
         * i.e. For lines, it should return how many lines; for TriangleMesh,
         * it should return how many triangle faces; for QuadMesh, it should 
         * return how many quad polygons.
         * @return Return the total amount of primitives in the mesh.
         *******************************************************/
        virtual size_t GetPrimitiveCount() = 0;
        virtual Math::Vector3 GetFaceCentroid(u32) const { return {}; }
        virtual u32 GetPolygonIndex(u32, u8) const{ return 0; }
        virtual Math::Vector3 GetPolygonNormal(u32) const { return {}; }
	    /*******************************************************************
         * @brief Send the mesh data to OpenGL to "create" the mesh. This
         * function must be called ever once before rendering. If any of the 
         * mesh data changes, i.e. the data in the Vertex struct, you need
         * to call this function again to rebuild the mesh to render.
         * @remark For any class derived from mesh class, you need to "teach"
         * the program how to build VAO.
         ******************************************************************/
        virtual void Build() = 0;
        virtual bool IsBuilt() { return m_isBuilt; }
	    /*******************************************************************
         * @brief Send this mesh with its data to shader to render this frame.
         ******************************************************************/
        virtual void Render();

        /*******************************************************
         * @brief Calculate mesh bounding sphere for editor selection
         *******************************************************/
        virtual void CalculateBoundingSphere(){}
        BoundingSphere const& GetBoundingSphere() const { return m_boudingSphere; }

        std::string const& GetLabel() const { return m_label; }
        void SetLabel(std::string const& label) { m_label = label; }
        virtual void Reflect(TwBar* editor, std::string const& groupName, GraphicsEngine* graphics);

    protected:
        std::string m_label;
        bool m_isBuilt = false;

        std::shared_ptr<VertexArrayObject> m_vertexArrayObject;
        BoundingSphere m_boudingSphere;//used only for ray casting selection for now.
    };
}
