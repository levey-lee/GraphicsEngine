#pragma once
#include "core/ComponentBase.h"
#include "graphics/Mesh.h"
#include "graphics/ShaderManager.h"

namespace Graphics
{
	class Material;
}

namespace Component
{
    /*******************************************************
	 * @brief This component sets mesh and material of the
	 * owner object. It is shaded because materials and meshes
	 * are set for each object each frame. We COULD optimize 
	 * this so that same materials and meshes are placed together
	 * so we don't have to set them for each object.
	 *******************************************************/
	class Renderer
		: public ComponentBase<Renderer, SHADED>
	{
	public:
		Renderer(std::shared_ptr<Graphics::Material> mat,
		         std::vector<std::shared_ptr<Graphics::Mesh> > meshes = std::vector<std::shared_ptr<Graphics::Mesh> >(),
		         bool defaultEnable = true);

        Renderer(std::shared_ptr<Graphics::Material> mat,
            std::shared_ptr<Graphics::Mesh> mesh,
            bool defaultEnable = true);

		void Start() override;
		void Update(float dt) override;
		void SetShaderParams(std::shared_ptr<Graphics::ShaderProgram> shader, 
            Graphics::GraphicsEngine* g) override;

		std::shared_ptr<Graphics::Material> GetMaterial() const { return m_material; }
	    /*******************************************************
	     * @brief Get a mesh in renderer's mesh container by indexing.
	     *  This should always be used rather than using a string label
	     *  for a liner search.
	     * @param index Index of the vector index to get the mesh.
	     * @return A pointer to the mesh.
	     *******************************************************/
	    std::shared_ptr<Graphics::Mesh> GetMesh(size_t index) const;
	    /*******************************************************
	     * @brief Linear search for a mesh in renderer's mesh container.
	     * @param label Label of the mesh used to search
	     * @return A pointer to the mesh, nullptr of failuer
	     *******************************************************/
	    std::shared_ptr<Graphics::Mesh> GetMesh(std::string const& label) const;
        size_t GetMeshSlotCount() const { return m_meshes.size(); }
        bool HasMesh(size_t& index) const;
        bool HasMaterial() const { return m_material != nullptr; }

		Renderer& AssignMaterial(std::shared_ptr<Graphics::Material> mat);
	    /*******************************************************
		 * @brief Add one mesh to Renderer. Call multiple times
		 * to add multiple meshes.
		 * @param mesh A pointer to a mesh.
		 * @return this reference.
		 *******************************************************/
		Renderer& AttachMesh(std::shared_ptr<Graphics::Mesh> mesh);
		Renderer& ReplaceMesh(size_t meshId, std::shared_ptr<Graphics::Mesh> mesh);
        
        REGISTER_EDITOR_COMPONENT(Renderer)
		void Reflect(TwBar* editor, std::string const& barName, std::string const& groupName,
            Graphics::GraphicsEngine* graphics) override;

	protected:
		void OnMaterialChanged();
		void OnMeshChanged();


		///object material that will be set to shader
		std::shared_ptr<Graphics::Material> m_material;

		///meshes used for the object, can be multiple meshes
        std::vector<std::pair<bool /*IfRender*/, std::shared_ptr<Graphics::Mesh> > > m_meshes;
	    std::shared_ptr<Graphics::Material> mat;

	private://this field is used for editor
        /////////////////////////////////////////
        /// Mesh Editor fields
        void resetMeshEditor(std::string const& oldMatGroupName, size_t meshId);
        static void TW_CALL GetMeshCB(void* value, void* clientData);
        static void TW_CALL SetMeshCB(const void* value, void* clientData);
        struct EditorMeshEnumWrapper
        {
            EditorMeshEnumWrapper(Renderer * _self=nullptr, size_t _enumIndex=0, size_t meshid=0)
            :self(_self), enumIndex(_enumIndex), meshId(meshid){}
            Renderer * self;
            size_t enumIndex;
            size_t meshId;
        };
        std::unordered_map<size_t, EditorMeshEnumWrapper> m_meshEnumWrappers;
         
        /////////////////////////////////////////
        /// Material Editor fields
        void resetMaterialEditor(std::string const& oldMatGroupName);
        static void TW_CALL GetMaterialCB(void* value, void* clientData);
        static void TW_CALL SetMaterialCB(const void* value, void* clientData);
        size_t m_materialEnumVal = 0;

#ifdef _DEBUG//used to debug warning print so that warning will only appear once
        bool m_hasMaterial = true;
        bool m_hasMesh = true;
#endif // _DEBUG

	};
}
