#include "Precompiled.h"
#include "core/HierarchicalObjectHandler.h"
#include "framework/Debug.h"


HierarchicalObjectHandlerNode* HierarchicalObjectHandler::AddRootObject(ObjectHandle object)
{
    Assert(m_roots.find(object.GetId()) == m_roots.end(), "Object already exist in HierarchicalObjectHandler.");
    m_roots.emplace(object.GetId(), HierarchicalObjectHandlerNode(object));
    return &m_roots[object];
}

HierarchicalObjectHandlerNode* HierarchicalObjectHandler::AttachNewChild(ObjectHandle parent, ObjectHandle newChild)
{
    Assert(newChild != -1, "Trying to add a null object as child.");
    for (auto& i : m_roots)
    {
        HierarchicalObjectHandlerNode* parrentNode = nullptr;
        parrentNode = i.second.m_objectHandle == parent
            ? &i.second
            : findChildNode(&i.second, parent);
        if (parrentNode)
        {
            return attachNewChild(parrentNode, newChild);
        }
    }
    throw ObjectErrorException(std::string("Invalid parent object handle to add new child."));
}

void HierarchicalObjectHandler::RemoveAllChildren(ObjectHandle parent)
{
    for (auto& i : m_roots)
    {
        HierarchicalObjectHandlerNode* parrentNode = nullptr;
        parrentNode = i.second.m_objectHandle == parrentNode->m_objectHandle
            ? &i.second
            : findChildNode(&i.second, parent);
        if (parrentNode)
        {
            return i.second.m_children.clear();
        }
    }
    throw ObjectErrorException(std::string("Invalid parent object handle to remove all children."));
}

void HierarchicalObjectHandler::RemoveAllChildren(HierarchicalObjectHandlerNode* parent)
{
    parent->m_children.clear();
}

void HierarchicalObjectHandler::RemoveChild(HierarchicalObjectHandlerNode* parentNode, ObjectHandle child)
{
    removeChild(parentNode, child);
}

void HierarchicalObjectHandler::RemoveChild(HierarchicalObjectHandlerNode* parentNode,
    HierarchicalObjectHandlerNode* childNode)
{
    removeChild(parentNode, childNode->m_objectHandle);
}

void HierarchicalObjectHandler::RemoveChildInParent(HierarchicalObjectHandlerNode* childNode)
{
    removeChild(childNode->m_parent, childNode->m_objectHandle);
}

void HierarchicalObjectHandler::RemoveChild(ObjectHandle parent, ObjectHandle child)
{
    //todo redo
    Assert(child != -1, "Trying to detach a null object as child.");
    for (auto& i : m_roots)
    {
        HierarchicalObjectHandlerNode* parrentNode = nullptr;
        parrentNode = i.second.m_objectHandle == parrentNode->m_objectHandle
            ? &i.second
            : findChildNode(&i.second, parent);
        if (parrentNode)
        {
            removeChild(parrentNode, child);
            return;
        }
    }
    throw ObjectErrorException(std::string("Invalid parent object handle to remove child."));
}

void HierarchicalObjectHandler::RemoveChildInParent(ObjectHandle child)
{
    Assert(child != -1, "Trying to detach a null object as child.");

    for (auto& i : m_roots)
    {
        HierarchicalObjectHandlerNode* childNode = nullptr;
        childNode = i.second.m_objectHandle == childNode->m_objectHandle
            ? &i.second
            : findChildNode(&i.second, child);
        if (childNode)
        {
            RemoveChildInParent(childNode);
            return;
        }
    }
    throw ObjectErrorException(std::string("Invalid parent object handle to Detach Child."));
}

HierarchicalObjectHandlerNode* HierarchicalObjectHandler::findChildNode(HierarchicalObjectHandlerNode* rootNode, ObjectHandle targetObj)
{
#if 1//dfs on all children using umap.find
    Assert(rootNode != nullptr, "Invalid root node to find object.");
    Assert(targetObj != -1, "Trying to find a null object.");
    auto objIter = rootNode->m_children.find(targetObj);
    if (objIter != rootNode->m_children.end())
        return &objIter->second;

    for (auto& i : rootNode->m_children)
    {
        HierarchicalObjectHandlerNode* targetNode = findChildNode(&i.second, targetObj);
        if (targetNode)
        {
            return targetNode;
        }
    }
    return nullptr;
#else//dfs on each node element
    if (rootNode->m_objectHandle == targetObj)
    {
        return rootNode;
    }
    HierarchicalObjectHandlerNode* nodeFound = nullptr;
    for (auto& i : rootNode->m_children)
    {
        nodeFound = findChildNode(&i.second, targetObj);
        if (nodeFound)
        {
            return nodeFound;
        }
    }
    return nodeFound;
#endif // 0

}

HierarchicalObjectHandlerNode* HierarchicalObjectHandler::attachNewChild(HierarchicalObjectHandlerNode* parent, ObjectHandle object)
{
    if (parent)
    {
        Assert(object != -1, "Trying to attach a null object.");
        parent->m_children.emplace(object, HierarchicalObjectHandlerNode(object, parent));
        return &parent->m_children[object];
    }
    throw ObjectErrorException(std::string("Invalid parent object handle to attach Child."));
}

void HierarchicalObjectHandler::removeChild(HierarchicalObjectHandlerNode* parent, ObjectHandle child)
{
    Assert(child != -1, "Trying to detach a null object.");
    if (parent)
    {
        if (parent->m_children.erase(child) == 0)
        {
            throw ObjectErrorException(std::string("Parent node doesn't contain child node."));
        }
        return;
    }
    throw ObjectErrorException(std::string("Invalid parent object handle to detach child."));
}
