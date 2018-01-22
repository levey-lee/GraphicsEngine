#pragma once

#include "core/Object.h"
struct HierarchicalObjectHandlerNode
{
    HierarchicalObjectHandlerNode() {}
    HierarchicalObjectHandlerNode(ObjectHandle handle, HierarchicalObjectHandlerNode* parent = nullptr)
        : m_objectHandle(handle), m_parent(parent) {}

    ObjectHandle m_objectHandle;
    HierarchicalObjectHandlerNode* m_parent = nullptr;
    std::unordered_map<ObjectId, HierarchicalObjectHandlerNode> m_children;
};



/*******************************************************
 * @brief A vector of trees to handle all the objects
 * with their children. This is basically used by transform
 * to update children's world transformation matrix.
 *******************************************************/
class HierarchicalObjectHandler
{
public:
    HierarchicalObjectHandlerNode* AddRootObject(ObjectHandle object);
    HierarchicalObjectHandlerNode* AttachNewChild(ObjectHandle parent, ObjectHandle newChild);
    //todo void ChangeParent(ObjectHandle newParrent, ObjectHandle currentParrent, ObjectHandle existingChild);
    void RemoveAllChildren(ObjectHandle parent);
    static void RemoveAllChildren(HierarchicalObjectHandlerNode* parent);
    void RemoveChild(ObjectHandle parent, ObjectHandle child);
    static void RemoveChild(HierarchicalObjectHandlerNode* parentNode, ObjectHandle child);
    static void RemoveChild(HierarchicalObjectHandlerNode* parentNode, HierarchicalObjectHandlerNode* childNode);
    void RemoveChildInParent(ObjectHandle child);
    static void RemoveChildInParent(HierarchicalObjectHandlerNode* childNode);
    auto& GetRootsRef() { return m_roots; }
private:
    static HierarchicalObjectHandlerNode* findChildNode(HierarchicalObjectHandlerNode* rootNode, ObjectHandle targetObj);
    //static HierarchicalObjectHandlerNode* findParentNode(HierarchicalObjectHandlerNode* rootNode, ObjectHandle child);
    static HierarchicalObjectHandlerNode* attachNewChild(HierarchicalObjectHandlerNode* rootNode, ObjectHandle child);
    static void removeChild(HierarchicalObjectHandlerNode* parent, ObjectHandle child);

    std::unordered_map<ObjectId, HierarchicalObjectHandlerNode> m_roots;
};


