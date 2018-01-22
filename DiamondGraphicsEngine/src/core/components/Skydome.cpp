#include "Precompiled.h"
#include "core/components/Skydome.h"
#include "core/components/Transform.h"
#include "graphics/Mesh.h"
#include "graphics/Materials.h"
#include "graphics/GraphicsEngine.h"

namespace TwCallBack
{
    //void TW_CALL GetSkydomeEnabled(void *value, void *clientData)
    //{
    //    Component::Skydome* skydome = static_cast<Component::Skydome*>(clientData);
    //    *static_cast<bool*>(value) = skydome->IsEnabled();
    //}
    //void TW_CALL SetSkydomeEnabled(const void *value, void *clientData)
    //{
    //    Component::Skydome* skydome = static_cast<Component::Skydome*>(clientData);
    //    skydome->SetEnabled(*static_cast<const bool*>(value));
    //}
}
namespace Component
{
    //void Skydome::SetShaderParams(std::shared_ptr<Graphics::ShaderProgram> shaderProgram, Graphics::GraphicsEngine* g)
    //{
    //    if (!m_isEnabled)
    //    {
    //        return;
    //    }
    //    if (m_mesh != nullptr)
    //    {
    //        m_mesh->Render();
    //    }
    //    if (m_material != nullptr)
    //    {
    //        m_material->SetShaderParameters(shaderProgram, g);
    //    }
    //}

    //void Skydome::Reflect(TwBar* editor, std::string const& /*barName*/, std::string const& groupName,
    //    Graphics::GraphicsEngine* /*graphics*/)
    //{
    //    std::string groupStr = "group=" + groupName;
    //    TwAddVarCB(editor, nullptr, TW_TYPE_BOOLCPP, 
    //        TwCallBack::SetSkydomeEnabled, TwCallBack::GetSkydomeEnabled, this, (groupStr+" label='Enabled'").c_str());
    //}

    //void Skydome::AssignMesh(std::shared_ptr<Graphics::Material> pmat)
    //{
    //    m_material = std::move(pmat);
    //}

    //void Skydome::AssignMesh(std::shared_ptr<Graphics::Mesh> pmesh)
    //{
    //    m_mesh = std::move(pmesh);
    //}
}

