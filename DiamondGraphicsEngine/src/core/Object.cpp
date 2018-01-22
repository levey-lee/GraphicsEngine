#include "Precompiled.h"
#include "core/Object.h"
#include "core/Scene.h"
#include "core/ComponentBase.h"
#include "core/components/Renderer.h"
#include "core/components/Transform.h"

namespace
{
    void TW_CALL GetObjectParentID(void *value, void *clientData)
    {
        Object* currentObj = static_cast<Object*>(clientData);
        if (currentObj->GetParent())
        {
            *static_cast<ObjectId *>(value) = currentObj->GetParent()->GetHandle();
        }
        else
        {
            *static_cast<ObjectId *>(value) = -1;
        }
    }
}

void Object::RemoveAllComponents() const
{//todo - remove all component from all the component pool with this ObjectId
}

void Object::Active()
{//todo - call enabled components' Start()
    m_isActive = true;
}

void Object::Deactive()
{//todo - not sure, should disable all components?
    m_isActive = false;
}

void Object::PushShadedComponent(ComponentInterface* comp, Graphics::ShaderType shaderType)
{
    if (m_shaderType == Graphics::ShaderType::Null)
    {
        m_shaderType = shaderType;
    }
    if (m_shaderType != Graphics::ShaderType::Null)
    {
        m_shadedComponents.push_back(comp);
    }
}

void Object::Reflect(TwBar* editor, std::string const& barName, Graphics::GraphicsEngine* graphics)
{
    TwAddSeparator(editor, nullptr, nullptr);
    TwAddButton(editor, nullptr, nullptr, nullptr, "label='Object Information'");
    TwAddVarCB(editor, "Parent ID", TW_TYPE_INT32, nullptr, GetObjectParentID, this, "group=Object");
    TwAddVarRO(editor, "Object ID", TW_TYPE_INT32, &m_objectHandle.m_objectId, "group=Object");
    TwAddVarRW(editor, "Object Name", TW_TYPE_STDSTRING, &m_name, "group=Object");
    TwAddVarRW(editor, "Active", TW_TYPE_BOOLCPP,  &m_isActive, "group=Object");
    //TwAddVarRO(editor, nullptr, TW_TYPE_INT32,  &m_shaderType, "label='Shader Type Enum' group=Object");
    TwAddSeparator(editor, nullptr, nullptr);
    TwAddButton(editor, nullptr, nullptr, nullptr, "label='Components'");
    for (auto& i : m_editorComponents)
    {
        i->Reflect(editor, barName, i->GetComponentTypeName(), graphics);
    }

}

BoundingSphere Object::GetBoundingSphere()
{
    using namespace Math;
    BoundingSphere sphere;
    if (HasComponent<Component::Renderer>())
    {
        Component::Renderer& renderer = GetComponentRef<Component::Renderer>();
        size_t meshId = 0;
        if (renderer.HasMesh(meshId))
        {            
            sphere = renderer.GetMesh(meshId)->GetBoundingSphere();
            Component::Transform& trans = GetComponentRef<Component::Transform>();
            Matrix4 worldTrans = trans.GetWorldTransform();

            Vector3 aPointOnSphere = sphere.radius*Vector3(1, 0, 0) + sphere.center;
            sphere.center = TransformPoint(worldTrans, sphere.center);
            Vector3 transformedPointOnSphere = TransformPoint(worldTrans, aPointOnSphere);

            sphere.radius = (sphere.center - transformedPointOnSphere).Length();
        }
    }
    return sphere;
}


void Object::AssignHierarchicalObjectHandlerNode(HierarchicalObjectHandlerNode* node)
{
    m_hohNode = node;
}

void Object::AssignScene(Scene* scene)
{
    m_scene = scene;
}

void Object::OnActive()
{//todo
}

void Object::OnDeactive()
{//todo
}
