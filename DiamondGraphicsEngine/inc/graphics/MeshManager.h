#ifndef H_MESH_LOADER
#define H_MESH_LOADER

namespace Math {
    struct Vector3;
}

namespace Graphics
{
    class Mesh;
    class TriangleMesh;

    /***************************************************************************
    * @brief
    * This class is responsible for loading new meshes from a file, using the
    * Wavefront OBJ file format. The implementation of this class is not fully
    * provided for you, so you are therefore expected to implement it for
    * assignment 1. Refer to TriangleMesh.h and the example Main file for how
    * constructing a TriangleMesh works. Bear in mind you only need to be able
    * to load the initial meshes provided (plane obj files that don't have uv,
    * normal etc.), but it may be worthwhile to implement a more complete
    * implementation that can support all sorts of OBJ files. The format itself
    * is very well described here:
    *   http://en.wikipedia.org/wiki/Wavefront_.obj_file
    **************************************************************************/
    class MeshManager
    {
    public:

        /*******************************************************
         * @brief Having a in-class triangle mesh handler is to
         * seperate mesh types. You can have as many as the small
         * class you want such as LineMesh, QuadMesh, MyBeautifulMesh
         * etc.
         *******************************************************/
        class TriangleMeshHandler
        {
        public:

            //todo recursively build menger sponge by dividing each quad face into 9 quad faces.
            //static std::shared_ptr<TriangleMesh> BuildMengerSponge(
            //    int level, 
            //    int levelMax, 
            //    std::vector<Vertex>& vertices,
            //    std::vector<unsigned int>& indices
            //);

            /*******************************************************
             * @brief Build a triangle manually. Right-hand system.
             * @return A shared pointer to the newly created mesh.
             *******************************************************/
            std::shared_ptr<TriangleMesh> BuildTriangle(std::string const &meshLabel);
            /*******************************************************
             * @brief Build a sphere mesh manually. Right-hand system.
             * @return A shared pointer to the newly created mesh.
             * @note It has a hole at poles.
             *******************************************************/
            std::shared_ptr<TriangleMesh> BuildSphere(std::string const &meshLabel);
            /*******************************************************************
            * @brief
            * Loads a new TriangleMesh from a Wavefront OBJ file, given that files
            * relative path. The path is relative to all files within assets/models
            * from the root directory of the project.
            * @param meshLabel The label of the mesh, used by editor.
            * @param objFileName Relative file name in the assets/models folder.
            * @return A shared pointer to the newly created mesh.
            ******************************************************************/
            std::shared_ptr<TriangleMesh> LoadObjMesh(std::string const &meshLabel, std::string const &objFileName);
        }m_TriangleMeshHandler;

        /*******************************************************
         * @brief Get all meshes. Use vector for editor enum to index.
         * @return A vector of all mesh pointers.
         *******************************************************/
        std::vector<std::shared_ptr<Mesh> > GetAllMeshes();
    private:
        static std::unordered_map<std::string /*label*/, std::shared_ptr<Mesh> >m_meshes;
    };
}

#endif