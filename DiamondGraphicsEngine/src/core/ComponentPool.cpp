#include "Precompiled.h"
#include "core/ComponentPool.h"

std::unordered_map<std::type_index, ComponentPoolManager*> ComponentPoolManager::m_pools = std::unordered_map<std::type_index, ComponentPoolManager*>();
std::map<UpdateOrder, std::vector<TypeIndex> > ComponentPoolManager::m_updateOrder;


void ComponentPoolManager::UpdateAllComponentPools(Scene* scene, float dt)
{
    for (auto& i : m_updateOrder)
    {
        for (auto& j : i.second)
        {
            m_pools[j]->UpdateThisPool(scene, dt);
        }
    }
}

//void ComponentPoolManager::RenderAllComponentPools(Scene* scene, std::shared_ptr<Graphics::ShaderProgram> shader)
//{
//    for (auto& i : m_updateOrder)
//    {
//        for (auto& j : i.second)
//        {
//            m_pools[j]->RenderThisPool(scene, shader);
//        }
//    }
//}

void ComponentPoolManager::StartAllComponentPools(Scene* scene)
{
    for (auto& i : m_updateOrder)
    {
        for (auto& j : i.second)
        {
            m_pools[j]->StartThisPool(scene);
        }
    }
}

void ComponentPoolManager::ClearAllComponentPools()
{
    for (auto& i : m_pools)
    {
        if (i.second)
        {
            delete i.second;
            i.second = nullptr;
        }
    }
    m_updateOrder.clear();
    m_pools.clear();
}

template <typename TComp>
void ComponentPoolManager::ChangeComponentUpdateOrder(UpdateOrder updateorder)
{
    auto& type_id = typeid(ComponentPool<TComp>);
    for (auto& i : m_updateOrder)
    {
        for (auto j = i.second.begin(); j != i.second.end(); ++j)
        {
            if (*j == type_id)
            {
                i.second.erase(j);
                m_updateOrder[updateorder].push_back(type_id);
                return;
            }
        }
    }
    std::string errmsg("trying to change component update order but the component bus does not exist. The type of the component is: ");
    errmsg += std::string(type_id.name());
    Assert(false, errmsg.data());
}

template <typename TComp>
ComponentPool<TComp>* ComponentPoolManager::GetPool()
{
    return static_cast<ComponentPool<TComp>*>(m_pools[typeid(ComponentPool<TComp>)]);
}

template <typename TComp>
void ComponentPoolManager::RemoveComponent(ObjectId id)
{//todo
}


void ComponentPoolManager::RemoveAllComponent(ObjectId id)
{
}
