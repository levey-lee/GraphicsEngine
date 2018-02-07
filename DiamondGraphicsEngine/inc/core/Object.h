#pragma once
#include "core/ComponentPool.h"
#include "core/BoundingSphere.h"
#include "graphics/ShaderManager.h"

class ComponentEditorInterface;

namespace Graphics {
    enum class ShaderType;
    class GraphicsEngine;
}

struct HierarchicalObjectHandlerNode;
class Object;
class ComponentInterface;
using RenderObject = std::vector<ComponentInterface*>;
/**
 * @brief 
 */
class ObjectErrorException
    : public std::exception
{
public:
    explicit ObjectErrorException(std::string msg): m_msg(std::move(msg)){}
    char const* what() const override { return m_msg.c_str(); }
private:
    std::string m_msg;
};


/**
 * @brief 
 */
struct ObjectHandle
{
    //this is needed since the class has non-default constructor
    ObjectHandle() = default;
    explicit ObjectHandle(ObjectId object_id) : m_objectId(object_id){}
    operator ObjectId() const { return m_objectId; }
    ObjectId GetId() const { return m_objectId; }

    bool operator<(ObjectHandle rhs) const { return m_objectId < rhs.m_objectId; }
private:
    friend class Object;
    ObjectId m_objectId = -1;
};

/**
 * @brief 
 */
class Object
{
    friend class Scene;
public:
    Object() = default;
    Object(ObjectId id, bool defaultActive = true) :m_isActive(defaultActive), m_objectHandle(ObjectHandle(id)){}
    virtual ~Object(){}
    ObjectHandle GetHandle() const { return m_objectHandle; }
    
    template <typename TComp> TComp& AddComponent();
    template <typename TComp, typename... Args> TComp& AddComponent(Args&&... args);
    template <typename TComp> TComp& GetComponentRef();
    template <typename TComp> const TComp& GetComponentRef() const;
    template <typename TComp> bool HasComponent() const;
    template <typename TComp> void RemoveComponent();

    void RemoveAllComponents() const;

    void Active();
    void Deactive();
    bool IsActive() const { return m_isActive; }

    HierarchicalObjectHandlerNode* GetHierarchicalObjectHandlerNode() const { return m_hohNode; }

    Scene* GetScene() const { return m_scene; }

    RenderObject* GetShadedComponents() { return &m_shadedComponents; }
    void PushShadedComponent(ComponentInterface* comp, Graphics::ShaderType shaderType);

    Object* GetParent() const { return m_parent; }

	std::string const& GetName() const { return m_name; }
	void SetName(std::string const& name) { m_name = name; }
	void Reflect(TwBar* editor, std::string const& barName, Graphics::GraphicsEngine* graphics);
    BoundingSphere GetBoundingSphere();
    const Graphics::ShaderType& GetShaderType() const { return m_shaderType; }
protected:
    void AssignHierarchicalObjectHandlerNode(HierarchicalObjectHandlerNode* node);
    void AssignScene(Scene* scene);
    void AssignParent(Object* parent) { m_parent = parent; }

    virtual void OnActive();
    virtual void OnDeactive();

	std::string m_name;
	std::vector<ComponentEditorInterface*> m_editorComponents;

    //container of components that are specified 'Shaded'
    RenderObject m_shadedComponents;
    Graphics::ShaderType m_shaderType = Graphics::ShaderType::Null;

    Object* m_parent = nullptr;

    bool m_isActive = true;
private:
    Scene* m_scene = nullptr;
    HierarchicalObjectHandlerNode* m_hohNode = nullptr;
    ObjectHandle m_objectHandle;

};


template <typename TComp>
TComp& Object::AddComponent()
{
    DEBUG_PRINT_DATA_FLOW
    ComponentPool<TComp>::AddComponent(m_objectHandle);
    auto& compRef = GetComponentRef<TComp>();
    compRef.AssignOwner(this);

	m_editorComponents.push_back(&compRef);

    return compRef;
}

template <typename TComp, typename ... Args>
TComp& Object::AddComponent(Args&&... args)
{
    DEBUG_PRINT_DATA_FLOW
    ComponentPool<TComp>::AddComponent(m_objectHandle, std::forward<Args>(args)...);
    auto& compRef = GetComponentRef<TComp>();
    compRef.AssignOwner(this);

	m_editorComponents.push_back(&compRef);

    return compRef;
}

template <typename TComp>
TComp& Object::GetComponentRef()
{
    return ComponentPool<TComp>::GetComponentRef(m_objectHandle);
}

template <typename TComp>
const TComp& Object::GetComponentRef() const
{
    return ComponentPool<TComp>::GetComponentRef(m_objectHandle);
}

template <typename TComp>
bool Object::HasComponent() const
{
    DEBUG_PRINT_DATA_FLOW
    return ComponentPool<TComp>::HasComponent(m_objectHandle);
}

