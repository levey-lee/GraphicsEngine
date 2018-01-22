#pragma once


namespace Graphics
{
    class GraphicsEngine;
    class Material;

	/*******************************************************************
     * @brief A simple manager to hold all the materials in the engine.
     ******************************************************************/
    class MaterialManager
    {
    public:
	    /*******************************************************************
         * @brief Load all materials in one file.
         * @param fileName Relative mtl file name in Asset/materials/.
         ******************************************************************/
        void LoadMaterials(std::string const& fileName, std::shared_ptr<GraphicsEngine> g);
        /*******************************************************
         * @brief Add material to manager, usually hardcoded at C++ side.
         * @param material Material pointer.
         * @return The same pointer as argument passed in.
         *******************************************************/
        std::shared_ptr<Material> AddMaterial(std::shared_ptr<Material> material);
        
        std::shared_ptr<Material> GetMaterial(std::string const& matName);
        
        /*******************************************************
         * @brief Retrieve all materials. Used by editor.
         * @return A vector of all materials by value. Linear copy.
         * @note Use vector to be editor friendly.
         *******************************************************/
        std::vector<std::shared_ptr<Material> > GetAllMaterials() const;
    private:
        /*******************************************************
         * @brief This is the helper function to put the material in the container to avoid 
         * string name mismatch.
         * @param matName Name that will be used for the material
         * @param material A pointer to a material, loaded from file
         *******************************************************/
        void emplaceMaterial(std::string const& matName, std::shared_ptr<Material> material);
        std::unordered_map<std::string/*name*/, std::shared_ptr<Material> > m_materials;
    };


}
