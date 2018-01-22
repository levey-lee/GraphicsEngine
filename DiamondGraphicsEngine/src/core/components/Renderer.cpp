#include "Precompiled.h"
#include "core/components/Renderer.h"
#include "graphics/Materials.h"
#include "graphics/Mesh.h"
#include "core/Scene.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/MaterialManager.h"
#include "graphics/MeshManager.h"

////////////////////////////////////////////
//  used for editor
////////////////////////////////////////////
namespace
{
    std::vector<std::shared_ptr<Graphics::Material> > materialsCache;
    std::vector<std::shared_ptr<Graphics::Mesh> > meshesCache;
    TwBar* editorCache;
    std::string barNameCache;
    std::string groupNameCache;
    Graphics::GraphicsEngine* graphicsCache;
}

namespace TwCallBack
{
    void TW_CALL GetEnabled(void* value, void* clientData)
    {
        Component::Renderer* renderer = static_cast<Component::Renderer*>(clientData);
        *static_cast<bool*>(value) = renderer->IsEnabled();

    }
    void TW_CALL SetEnabled(const void* value, void* clientData)
    {
        Component::Renderer* renderer = static_cast<Component::Renderer*>(clientData);
        bool enabled = *static_cast<const bool*>(value);
        renderer->SetEnabled(enabled);
    }
}

Component::Renderer::Renderer(
    std::shared_ptr<Graphics::Material> mat,
    std::vector<std::shared_ptr<Graphics::Mesh> > meshes,
    bool defaultEnable)
    : ComponentBase(defaultEnable), m_material(mat), mat(std::move(mat))
{
    for (size_t i = 0; i < meshes.size(); i++)
    {
        m_meshes[i].first = true;
        m_meshes[i].second = meshes[i];
        m_meshEnumWrappers.emplace(i, EditorMeshEnumWrapper(this, 0, i));
    }
}

Component::Renderer::Renderer(
    std::shared_ptr<Graphics::Material> mat,
    std::shared_ptr<Graphics::Mesh> mesh,
    bool defaultEnable)
    : ComponentBase(defaultEnable), m_material(std::move(mat))
{
    if (mesh)
    {
        m_meshes.emplace_back(true, mesh);
        m_meshEnumWrappers.emplace(0, EditorMeshEnumWrapper(this, 0, 0));
    }
}

void Component::Renderer::Start()
{
    DEBUG_PRINT_DATA_FLOW
}

void Component::Renderer::Update(float /*dt*/)
{
}

void Component::Renderer::SetShaderParams(std::shared_ptr<Graphics::ShaderProgram> shader,
                                          Graphics::GraphicsEngine* g)
{
    if (m_isEnabled == false)
    {
        return;
    }
#ifdef _DEBUG
    if (m_hasMaterial)
    {
        WarnIf(m_material == nullptr, "Object with id:[%d] and name:\"%s\" has renderer component but does not have material assigned.",
            static_cast<int>(m_owner->GetHandle().GetId()), m_owner->GetName().c_str());
        if (m_material == nullptr)
        {
            m_hasMaterial = false;
        }
    }
    if (m_hasMesh)
    {
        bool allNull = true;
        for (auto& i : m_meshes)
        {
            if (i.second != nullptr)
            {
                allNull = false;
                break;
            }
        }

        WarnIf(allNull, "Object with id:[%d] and name:\"%s\" has renderer component but does not have mesh attached.",
            static_cast<int>(m_owner->GetHandle().GetId()), m_owner->GetName().c_str());
        if (allNull)
        {
            m_hasMesh = false;
        }
    }
#endif // _DEBUG
    if (m_material != nullptr)
    {
        m_material->SetShaderParameters(shader, g);
#ifdef _DEBUG
        m_hasMaterial = true;
#endif // _DEBUG
    }
    if (!m_meshes.empty())
    {
        for (auto& i : m_meshes)
        {
            if (i.second != nullptr && i.first == true)
            {
                i.second->Render();
            }
        }
#ifdef _DEBUG

        bool allNull = true;
        for (auto& i : m_meshes)
        {
            if (i.second != nullptr)
            {
                allNull = false;
                break;
            }
        }
        if (allNull == false)
        {
            m_hasMesh = true;
        }
#endif // _DEBUG
    }
}


std::shared_ptr<Graphics::Mesh> Component::Renderer::GetMesh(size_t index) const
{
    Assert(index < m_meshes.size(), "Invalid index to get mesh.");
    return m_meshes[index].second;
}

std::shared_ptr<Graphics::Mesh> Component::Renderer::GetMesh(std::string const& label) const
{
    for (auto& i : m_meshes)
    {
        if (i.second->GetLabel()==label)
        {
            return i.second;
        }
    }
    Warning("Mesh with label \"%s\" does not exist in Renderer Component of object with index [%i] and name \"%s\". ",
        label.c_str(), m_owner->GetHandle().GetId(), m_owner->GetName().c_str());
    return nullptr;
}

bool Component::Renderer::HasMesh(size_t& index) const
{
    size_t id = 0;
    for (auto& i : m_meshes)
    {
        if (i.second != nullptr)
        {
            index = id;
            return true;
        }
        id++;
    }
    return false;
}

Component::Renderer& Component::Renderer::AssignMaterial(std::shared_ptr<Graphics::Material> mat)
{
    m_material = std::move(mat);
    OnMaterialChanged();
    return *this;
}

Component::Renderer& Component::Renderer::AttachMesh(std::shared_ptr<Graphics::Mesh> mesh)
{
    if (mesh)
    {
        Assert(mesh->IsBuilt(), "Trying to assign a mesh but it's not built.");
    }
    m_meshEnumWrappers.emplace(m_meshes.size(), EditorMeshEnumWrapper(this, 0, m_meshes.size()));
    m_meshes.emplace_back(true, std::move(mesh));
    OnMeshChanged();
    return *this;
}

Component::Renderer& Component::Renderer::ReplaceMesh(size_t meshId, std::shared_ptr<Graphics::Mesh> mesh)
{
    Assert(meshId < m_meshes.size(), "Invalid index to get mesh.");
    m_meshes[meshId].second = (mesh);
    return *this;
}

//void Component::Renderer::ChangeShaderType(Graphics::ShaderType shaderType)
//{
//    Graphics::ShaderType oldType = m_shaderType;
//    m_shaderType = shaderType;
//    m_owner->GetScene()->OnObjectShaderTypeChanged(m_owner, oldType, shaderType);
//    //todo change material when shader type changes
//}

void Component::Renderer::Reflect(TwBar* editor, std::string const& barName, std::string const& groupName,
    Graphics::GraphicsEngine* graphics)
{
    editorCache = editor;
    barNameCache = barName;
    groupNameCache = groupName;
    graphicsCache = graphics;

    std::string groupStr = "group=" + groupName;
    
    TwAddVarCB(editor, nullptr, TW_TYPE_BOOLCPP, TwCallBack::SetEnabled, TwCallBack::GetEnabled, this, (groupStr + " label=Enabled").c_str());

    ////////////////////////////////////
    //Change Mesh
    std::shared_ptr<Graphics::MeshManager> meshManager = graphics->GetMeshManager();
    meshesCache = meshManager->GetAllMeshes();
    size_t meshSize = meshesCache.size();
    for (size_t j = 0; j < m_meshes.size(); j++)
    {
        std::string enumStr = "None,";
        if (meshSize)
        {
            for (size_t i = 0; i < meshSize; ++i)
            {
                enumStr += meshesCache[i]->GetLabel();
                enumStr += ",";
                if (m_meshes[j].second == meshesCache[i])
                {
                    m_meshEnumWrappers[j].self = this;
                    m_meshEnumWrappers[j].meshId = j;
                    m_meshEnumWrappers[j].enumIndex = i + 1;//+1 since 0 is reserved for 'None'
                }
            }
        }

        std::string meshGroupStr = "Mesh[" + std::to_string(j) + "]";
        TwType meshType = TwDefineEnumFromString(nullptr, enumStr.c_str());
        TwAddVarCB(editor, nullptr, meshType, SetMeshCB, GetMeshCB, &m_meshEnumWrappers[j], (std::string("group=" + meshGroupStr) + " label='Select Mesh'").c_str());

        ////////////////////////////////////
        //Reflect mesh
        if (m_meshes[j].second != nullptr)
        {
            TwAddVarRW(editor, nullptr, TW_TYPE_BOOLCPP, &m_meshes[j].first, (std::string("group=" + meshGroupStr) + " label='Render'").c_str());
            m_meshes[j].second->Reflect(editor, meshGroupStr, graphics);
            std::string defStr = "'" + barName + "'/" + meshGroupStr + " group=Mesh";
            TwDefine(defStr.c_str());
        }        
    }
    if (!m_meshes.empty())
    {
        std::string defStr = "'" + barName + "'/" + "Mesh " + groupStr;
        TwDefine(defStr.c_str());
    }


    ////////////////////////////////////
    //Change Material
    std::shared_ptr<Graphics::MaterialManager> materialManager = graphics->GetMaterialManager();
    materialsCache = materialManager->GetAllMaterials();
    size_t matSize = materialsCache.size();
    std::string enumStr = "None,";
    if(matSize)
    {
        for (size_t i = 0; i < matSize; ++i)
        {
            enumStr += materialsCache[i]->GetMaterialName();
            enumStr += ",";
            if (materialsCache[i] == m_material)
            {
                m_materialEnumVal = i +1;//+1 since 0 is reserved for 'None'
            }
        }
    }
    TwType MatType = TwDefineEnumFromString(nullptr, enumStr.c_str());
    TwAddVarCB(editor, nullptr, MatType, SetMaterialCB, GetMaterialCB, this, (std::string("group=Material[0]") + " label='Select Material'").c_str());
    ////////////////////////////////////
    //Reflect material
    if (m_material != nullptr)
    {
        m_material->Reflect(editor, "Material[0]", graphics);
        std::string matDefStr = "'" + barName + "'/Material[0] " + " group=Material";
        TwDefine(matDefStr.c_str());

        matDefStr = "'" + barName + "'/Material " + groupStr;
        TwDefine(matDefStr.c_str());
    }
}


void Component::Renderer::OnMaterialChanged()
{    
}

void Component::Renderer::OnMeshChanged()
{
}

void Component::Renderer::resetMeshEditor(std::string const& oldMatGroupName, size_t meshId)
{
    if (!oldMatGroupName.empty())
    {
        TwRemoveVar(editorCache, oldMatGroupName.c_str());
    }
    //std::string enumStr = "None,";
    //if (meshSizeCache)
    //{
    //    for (size_t i = 0; i < meshSize; ++i)
    //    {
    //        enumStr += meshesCache[i]->GetLabel();
    //        enumStr += ",";
    //        if (m_meshes[j].second == meshesCache[i])
    //        {
    //            m_meshEnumWrappers[meshId].self = this;
    //            m_meshEnumWrappers[meshId].meshId = j;
    //            m_meshEnumWrappers[meshId].enumIndex = i + 1;//+1 since 0 is reserved for 'None'
    //        }
    //    }
    //}

    //std::string meshGroupStr = "Mesh[" + std::to_string(meshId) + "]";
    //TwType meshType = TwDefineEnumFromString(nullptr, enumStr.c_str());
    //TwAddVarCB(editor, nullptr, meshType, SetMeshCB, GetMeshCB, &m_meshEnumWrappers[j], (std::string("group=" + meshGroupStr) + " label='Select Mesh'").c_str());

    //if (m_meshes != nullptr)
    //{
    //    std::string groupStr = "group=" + groupNameCache;
    //    m_meshes->Reflect(editorCache, "Mesh", graphicsCache);
    //    std::string defStr = "'" + barNameCache + "'/Mesh " + groupStr;
    //    TwDefine(defStr.c_str());
    //}
}

void Component::Renderer::GetMeshCB(void* value, void* clientData)
{
    size_t meshEnumIndex = static_cast<EditorMeshEnumWrapper *>(clientData)->enumIndex;
    *static_cast<size_t *>(value) = meshEnumIndex;
}

void Component::Renderer::SetMeshCB(const void* value, void* clientData)
{
    EditorMeshEnumWrapper * meshWrapper = static_cast<EditorMeshEnumWrapper *>(clientData);
    size_t meshEnumIndex = *static_cast<const size_t *>(value);
    meshWrapper->self->m_meshEnumWrappers[meshWrapper->meshId].enumIndex = meshEnumIndex;
    size_t meshId = meshWrapper->meshId;


    if (meshEnumIndex)
    {
        meshWrapper->self->ReplaceMesh(meshId, meshesCache[meshEnumIndex - 1]);
        //meshWrapper->self->resetMeshEditor("Mesh["+std::to_string(meshId) + "]", meshId);
    }
    else
    {
        meshWrapper->self->ReplaceMesh(meshId, nullptr);
        //meshWrapper->self->resetMeshEditor("", meshId);
    }
}

void Component::Renderer::resetMaterialEditor(std::string const& oldMatGroupName)
{
    TwRemoveVar(editorCache, oldMatGroupName.c_str());

    std::string enumStr = "None,";
    size_t matSize = materialsCache.size();
    if (matSize)
    {
        for (size_t i = 0; i < matSize; ++i)
        {
            enumStr += materialsCache[i]->GetMaterialName();
            enumStr += ",";
            if (materialsCache[i] == m_material)
            {
                m_materialEnumVal = i + 1;//+1 since 0 is reserved for 'None'
            }
        }
    }
    TwType MatType = TwDefineEnumFromString(nullptr, enumStr.c_str());
    TwAddVarCB(editorCache, nullptr, MatType, SetMaterialCB, GetMaterialCB, this, (std::string("group=Material[0]") + " label='Select Material'").c_str());
    ////////////////////////////////////
    //Reflect material
    if (m_material != nullptr)
    {
        m_material->Reflect(editorCache, "Material[0]", graphicsCache);
        std::string matDefStr = "'" + barNameCache + "'/Material[0] " + " group=Material";
        TwDefine(matDefStr.c_str());
        matDefStr = "'" + barNameCache + "'/Material group=" + groupNameCache;
        TwDefine(matDefStr.c_str());
    }
}


void Component::Renderer::SetMaterialCB(const void* value, void* clientData)
{
    size_t matIndex = *static_cast<const size_t *>(value);
    Renderer * renderer = static_cast<Renderer *>(clientData);
    renderer->m_materialEnumVal = matIndex;
    if (matIndex)
    {
        renderer->AssignMaterial(materialsCache[matIndex - 1]);//-1 since 0 is reserved for 'None', but 'None' is not in the vector
        renderer->resetMaterialEditor("Material[0]");
    }
    else
    {
        renderer->AssignMaterial(nullptr);
        renderer->resetMaterialEditor("Material[0]");
    }
}

void Component::Renderer::GetMaterialCB(void* value, void* clientData)
{
    size_t matIndex = static_cast<Renderer *>(clientData)->m_materialEnumVal;
    *static_cast<size_t *>(value) = matIndex;
}

