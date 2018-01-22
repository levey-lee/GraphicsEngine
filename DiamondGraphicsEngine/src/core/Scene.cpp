#include "Precompiled.h"
#include "core/Scene.h"
#include "core/components/Transform.h"
#include "core/components/Renderer.h"

Scene::Scene()
{
}

Scene::~Scene()
{
    //todo clear all obj
}

Object& Scene::CreateObject(Graphics::ShaderType shaderType, bool defaultActive)
{
    //Assign object Id
    ObjectId thisObjId = m_nextFreeId++;

    //create object in the scene
    m_objects[thisObjId] = Object(thisObjId, defaultActive);

    Object& objRef = m_objects[thisObjId];

    //assign scene to obj
    objRef.AssignScene(this);

    //attach the new created object to base level of the tree
    objRef.AssignHierarchicalObjectHandlerNode(
        m_hierarchicalObjectHandler.AddRootObject(ObjectHandle(thisObjId)));

    //add object to editor
	m_editorObjects.push_back(&objRef);

    //create transform
    objRef.AddComponent<Component::Transform>(shaderType);

    //return the new created object as reference
    return objRef;
}

Object& Scene::CreateChildObject(ObjectHandle parent, Graphics::ShaderType shaderType, bool defaultActive)
{
    //Assign object Id
    ObjectId childObjId = m_nextFreeId++;

    //create object in the scene
    m_objects[childObjId] = Object(childObjId, defaultActive);

    Object& objRef = m_objects[childObjId];

    //assign scene to obj
    objRef.AssignScene(this);

    //attach the new created object to an object in the tree
    objRef.AssignHierarchicalObjectHandlerNode(
        m_hierarchicalObjectHandler.AttachNewChild(parent, ObjectHandle(childObjId)));
    
    objRef.AssignParent(&m_objects[parent]);

    //add object to editor
    m_editorObjects.push_back(&objRef);

    //create transform
    objRef.AddComponent<Component::Transform>(shaderType);

    //return the new child created
    return objRef;
}

Object& Scene::GetObjectRef(ObjectHandle handle)
{
    if (m_objects.find(handle) == m_objects.end())
    {
        throw ObjectErrorException("Trying to get an object that doesn't exist.");
    }
    return m_objects[handle.GetId()];
}

void Scene::DeleteObject(ObjectHandle handle)
{//todo
}

//void Scene::DeleteObject(const ObjectHandle& handle)
//{
//todo
//    m_hierarchicalObjectHandler.RemoveAllChildren(handle);
//    m_hierarchicalObjectHandler.RemoveChildInParent(handle);
//}

void Scene::StartScene()
{
    DEBUG_PRINT_DATA_FLOW
    initializeHierarchicalTransform();
    initializeRenderObjectList();

    ComponentPoolManager::StartAllComponentPools(this);
}

void Scene::UpdateScene(float dt)
{
    ComponentPoolManager::UpdateAllComponentPools(this, dt);
}


void Scene::initializeHierarchicalTransform()
{
    DEBUG_PRINT_DATA_FLOW
    using namespace Component;
    auto roots = m_hierarchicalObjectHandler.GetRootsRef();
    for (auto& i : roots)
    {
        //set all children's world matrices
        InitTransformTree(&i.second);
    }
}

void Scene::initializeRenderObjectList()
{
    DEBUG_PRINT_DATA_FLOW
    for (auto& i : m_objects)
    {
        Graphics::ShaderType objShaderType = i.second.GetShaderType();
        if (objShaderType != Graphics::ShaderType::Null)
        {
            RenderObject* shadedComponents = i.second.GetShadedComponents();
            m_renderObjectList[objShaderType][i.first] = shadedComponents;
        }
    }
}

//DFS traverse to set all children's transform
void Scene::InitTransformTree(HierarchicalObjectHandlerNode* node)
{
    using namespace Component;

    const ObjectHandle& hCurrentToSet = node->m_objectHandle;
    Object& curToSetRef = GetObjectRef(hCurrentToSet);
    if (node->m_parent)//the input node has parent node, concat matrices
    {
        Object& parentObjRef = GetObjectRef(node->m_parent->m_objectHandle);
        const Math::Matrix4& parentTransMatrix = parentObjRef.GetComponentRef<Transform>().GetWorldTransform();

        //set current node
        Transform& currTransToSet = curToSetRef.GetComponentRef<Transform>();
        currTransToSet.SetWorldTransform(parentTransMatrix * currTransToSet.CalcLocalTransform());
    }
    else//the input node is root, has no parent node
    {
        Transform& currTransToSet = curToSetRef.GetComponentRef<Transform>();
        currTransToSet.SetWorldTransform(currTransToSet.CalcLocalTransform());
    }

    ///dfs traverse all children
    for (auto& i : node->m_children)
    {
        InitTransformTree(&i.second);
    }
}

void Scene::UpdateTransformTree(HierarchicalObjectHandlerNode* node, Math::Matrix4 const& parentWorldMatrix)
{//BUG function transform rotation does not perform correctly
    using namespace Component;

    const ObjectHandle& hCurrentToSet = node->m_objectHandle;
    Object& curToSetRef = GetObjectRef(hCurrentToSet);

    //set current node
    Transform& currTransToSet = curToSetRef.GetComponentRef<Transform>();
    Math::Matrix4 newWorldMatrix = parentWorldMatrix * currTransToSet.GetLocalTransform();
    currTransToSet.SetWorldTransform(newWorldMatrix);

    for (auto& i : node->m_children)
    {
        UpdateTransformTree(&i.second, newWorldMatrix);
    }
}

void Scene::OnObjectShaderTypeChanged(Object* obj, Graphics::ShaderType oldType, Graphics::ShaderType newType)
{
    RenderObject* shadedComponents = obj->GetShadedComponents();

    auto shaderObjects = m_renderObjectList.find(oldType);
    if (shaderObjects != m_renderObjectList.end())
    {
        m_renderObjectList[oldType].erase(obj->GetHandle());
    }

    if ((shadedComponents->empty() == false)
        && (newType != Graphics::ShaderType::Null))
    {
        m_renderObjectList[newType][obj->GetHandle()] = shadedComponents;
    }
}

//void Scene::renderShadedComponents(Graphics::GraphicsEngine* graphics)
//{
//    std::shared_ptr<Graphics::ShaderManager> shaderManager
//        = graphics->GetShaderManager();
//    for (auto& i : m_renderObjectList)//per shader
//    {
//        //bind this shader to render all object with this shader type
//        auto shader = shaderManager->GetShader(i.first);
//        shader->Bind();
//
//        std::shared_ptr<Graphics::LightManager> lightManager = graphics->GetLightManager();
//        lightManager->SetLightsUniform(shader);
//
//        for (auto& j : i.second)//per object
//        {
//            for (auto& k : *j.second)//per shaded component
//            {
//                k->SetShaderParams(i.first, graphics);
//            }
//        }
//    }
//
//    shaderManager->UnbindAllShader();
//
//}
