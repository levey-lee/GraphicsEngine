#pragma once
#include "core/ComponentBase.h"
#include "graphics/ShaderManager.h"
#include "Renderer.h"

namespace Graphics {
    class Mesh;
    class Material;
}

namespace Component
{
    class Skydome
        : public Renderer
    {
    public:
        explicit Skydome(
            std::shared_ptr<Graphics::Material> mat,         
            std::shared_ptr<Graphics::Mesh> mesh,
            bool defaultEnable = true) : Renderer(mat, mesh, defaultEnable){}

  //      explicit Skydome(
  //          std::shared_ptr<Graphics::Material> mat,         
  //          std::shared_ptr<Graphics::Mesh> mesh,
  //          bool defaultEnable = true) : ComponentBase(defaultEnable), m_material(std::move(mat)), m_mesh(std::move(mesh)) {}

  //      void SetShaderParams(std::shared_ptr<Graphics::ShaderProgram>, Graphics::GraphicsEngine*) override;

        REGISTER_EDITOR_COMPONENT(Skydome)
		//void Reflect(TwBar* editor, std::string const& barName, std::string const& groupName, Graphics::GraphicsEngine* graphics) override;

  //      void AssignMesh(std::shared_ptr<Graphics::Material> pmat);
  //      void AssignMesh(std::shared_ptr<Graphics::Mesh> pmesh);

  //  private:

  //      std::shared_ptr<Graphics::Material> m_material;
  //      std::shared_ptr<Graphics::Mesh> m_mesh;
    };
}
