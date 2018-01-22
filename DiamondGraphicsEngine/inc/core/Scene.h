#pragma once
#include "core/Object.h"
#include "core/HierarchicalObjectHandler.h"
#include "graphics/ShaderManager.h"
namespace Math {
    struct Matrix4;
}

namespace Graphics {
    class GraphicsEngine;
}

using ObjectHashTable = std::unordered_map<ObjectId, Object>;
/****************************************************
 * @brief Scene is just a container of objects. Each
 * object should have its owner scene.
 * The framework is able to deal with multiple scenes
 * at the same time, but their objects cannot interact
 * with each other.
 ***************************************************/
class Scene
{
public:
    Scene();
    virtual ~Scene();
    /**************************************************
     * @brief Create an object with transform
     * @param defaultActive If the object is active by default.
     * @param shaderType Shader type of the object
     * @return reference to the new created object
     ***************************************************/
    virtual Object& CreateObject(Graphics::ShaderType shaderType = Graphics::ShaderType::Null, bool defaultActive = true);
    virtual Object& CreateChildObject(ObjectHandle parent, Graphics::ShaderType shaderType = Graphics::ShaderType::Null, bool defaultActive = true);
    virtual Object& GetObjectRef(ObjectHandle handle);
    virtual void DeleteObject(ObjectHandle handle);
    
    virtual void StartScene();
    virtual void UpdateScene(float dt);

    void InitTransformTree(HierarchicalObjectHandlerNode* root);
    //BUG function below does not transform rotation correctly
    void UpdateTransformTree(HierarchicalObjectHandlerNode* node, Math::Matrix4 const& parentWorldMatrix);
    void OnObjectShaderTypeChanged(Object* obj, Graphics::ShaderType oldType, Graphics::ShaderType newType);
    auto& GetRenderObjectListRef() { return m_renderObjectList; }

    std::vector<Object*>& GetEditorObjectRef() { return m_editorObjects; }

protected:
    //void renderShadedComponents(Graphics::GraphicsEngine* graphics);
    void initializeHierarchicalTransform();
    void initializeRenderObjectList();

    ObjectHashTable m_objects;
    HierarchicalObjectHandler m_hierarchicalObjectHandler;
    /*  two dimensional hash table of objects that are going to be rendered
     *       -------------
     *      |Shader Type 1| -> { (objectId, shaded components) ,(objectId, shaded components),...}
     *       -------------
     *      |Shader Type 2| -> { (objectId, shaded components) ,(objectId, shaded components),...}
     *       -------------
     *      |Shader Type 3| -> { (objectId, shaded components) ,(objectId, shaded components),...}
     *          ... ...
     */
    std::unordered_map<Graphics::ShaderType, std::unordered_map<ObjectId, RenderObject*> > m_renderObjectList;

private:
    ObjectId m_nextFreeId = 0;

	std::vector<Object*> m_editorObjects;
};




