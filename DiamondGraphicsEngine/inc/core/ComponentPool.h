#pragma once
#include <utility>
#include "framework/Debug.h"

////////////////////////////////////////////////////////////
////////////////    Definitions   //////////////////////////
////////////////////////////////////////////////////////////
static const size_t c_DefaultComponentReservedSize = 128U;
static const unsigned int c_DefaultUpdateOrder = 1000;
////////////////////////////////////////////////////////////
////////////////    Forward Declaration   //////////////////
////////////////////////////////////////////////////////////
using TypeIndex = std::type_index;
using UpdateOrder = unsigned;
template <typename T> class ComponentPool;

/*****************************************************************
 * @brief exception class used for component pool error
 *****************************************************************/
class ComponentPoolErrorException
    : public std::exception
{
    std::string m_msg;
public:
    explicit ComponentPoolErrorException(std::string  msg) : m_msg(std::move(msg)) {}
    char const* what() const override { return m_msg.c_str(); }
};

/*******************************************************************
 * @brief A static component manager used to update and store all
 * the component pools. Update function uses an order so each 
 * component will have different update order. By default, the have 
 * the same order, but user could change the order to manage component
 * pools.
 ******************************************************************/
class ComponentPoolManager
{
    friend class Scene;
public:
    //abstract class declaration
    virtual ~ComponentPoolManager() = 0 {}

	///virtual methods used for derived class, which are component pools
    virtual void StartThisPool(Scene* scene) = 0;
    virtual void UpdateThisPool(Scene* scene, float) = 0;

	/*******************************************************************
     * @brief This function changes the order to update of one component,
     * the higher the number is, the prior the component update will be 
     * called.
     * @tparam TComp The type of the Component class, NOT Component Pool
     * @param updateorder unsigned int for an update order of a component
     ******************************************************************/
    template <typename TComp> static void ChangeComponentUpdateOrder(UpdateOrder updateorder);
    template <typename TComp> static ComponentPool<TComp>* GetPool();

protected:
    static void CleanUp() { ClearAllComponentPools(); }
    static void UpdateAllComponentPools(Scene* scene, float dt);
    static void StartAllComponentPools(Scene* scene);
    static void ClearAllComponentPools();
    template <typename TComp> static void CreatePool(UpdateOrder updateorder = c_DefaultUpdateOrder);
    template <typename TPool> static void AddPool(TPool* pool, UpdateOrder updateorder = c_DefaultUpdateOrder);
    
    template <typename TComp> void RemoveComponent(ObjectId id);
    static void RemoveAllComponent(ObjectId id);

    static std::map<UpdateOrder, std::vector<TypeIndex> > m_updateOrder;
    static std::unordered_map<TypeIndex, ComponentPoolManager*> m_pools;
};


/*******************************************************************
 * @brief This manages all the components with the same type of TComp.
 * All the components with the same type are stored here.
 * Index of a component is an object id as its handle.
 * @tparam TComp The type of a specific component.
 ******************************************************************/
template <typename TComp>
class ComponentPool 
    : public ComponentPoolManager
{
    //todo Implement a ComponentPoolDebugger for visualization of component data
    //friend class ComponentPoolDebugger;
public:
    ComponentPool();


	/*******************************************************************
	* @brief This function will be called only once when Scene starts.
	* @param scene The scene that contains objects that are going to be
	* started.
	******************************************************************/
    void StartThisPool(Scene* scene) override;
	/*******************************************************************
     * @brief Update all the components in the scene. Component update
     * is based on component types, not objects. All the components with
     * the same type will be updated at the same time.
     * @remark The component architecture is designed by component types is for CPU
     * efficiency. Because they are more cache-friendly.
     * @param scene The scene that contains objects that are going to be
     * updated.
     * @param dt frame time in seconds
     ******************************************************************/
    void UpdateThisPool(Scene* scene, float dt) override;

    static TComp& GetComponentRef(ObjectId object);

    static bool HasComponent(ObjectId object);

    static void RemoveComponent(ObjectId object);

	/*******************************************************************
     * @brief Add a component to the pool, this function is used for 
     * Component classes that have constructor parameters.
     * @tparam Tparams Component class' constructor parameter types.
     * @param ObjectId Object id in the scene as handle.
     * @param params Component' constructor parameter values.
     ******************************************************************/
    template <typename... Tparams> 
    static void AddComponent(ObjectId ObjectId, Tparams&&... params);

	/*******************************************************************
     * @brief Add a component to the pool, this function is used for 
     * Component classes that have NO constructor parameters, which 
     * is the default constructor.
     * @param ObjectId Object id in the scene as handle.
     ******************************************************************/
    static void AddComponent(ObjectId ObjectId);

private:
    //dummy way to detect if we need to allocate memory for the pool
    static bool m_isInitialized;
    static std::unordered_map<ObjectId, TComp> m_components;
};

template <typename TComp> std::unordered_map<ObjectId, TComp> ComponentPool<TComp>::m_components;
template <typename TComp> bool ComponentPool<TComp>::m_isInitialized = false;

//===================================================================
//////////////////////   Implementation  ////////////////////////////
//===================================================================

template <typename TComp>
void ComponentPoolManager::CreatePool(UpdateOrder updateorder)
{
    DEBUG_PRINT_DATA_FLOW
    AddPool(SMART_NEW ComponentPool<TComp>(), updateorder);
}

template <typename TPool>
void ComponentPoolManager::AddPool(TPool* pool, UpdateOrder updateorder)
{
    DEBUG_PRINT_DATA_FLOW
    auto& type_id = typeid(pool);
    m_pools[type_id] = pool;
    m_updateOrder[updateorder].push_back(type_id);
}

template <typename TComp>
ComponentPool<TComp>::ComponentPool()
{
    DEBUG_PRINT_DATA_FLOW
    m_components.reserve(c_DefaultComponentReservedSize);
}

template <typename TComp>
template <typename ... Tparams>
void ComponentPool<TComp>::AddComponent(ObjectId ObjectId, Tparams&&... params)
{
    DEBUG_PRINT_DATA_FLOW
    if (m_isInitialized == false)
    {
        ComponentPoolManager::CreatePool<TComp>();
        m_isInitialized = true;
    }
    auto findcomp = m_components.find(ObjectId);
    if (findcomp != m_components.end())//component not found
    {
        std::string errmsg("trying to add ");
        errmsg += std::string(typeid(TComp).name());
        errmsg += " but it already exists.";
        Assert(false, errmsg.data());
        return;
    }
    m_components.emplace(ObjectId, TComp(params...));
}

template <typename TComp>
void ComponentPool<TComp>::AddComponent(ObjectId ObjectId)
{
    DEBUG_PRINT_DATA_FLOW
        if (m_isInitialized == false)
        {
            ComponentPoolManager::CreatePool<TComp>();
            m_isInitialized = true;
        }
    auto findcomp = m_components.find(ObjectId);
    if (findcomp != m_components.end())//component not found
    {
        std::string errmsg("trying to add component ");
        errmsg += std::string(typeid(TComp).name());
        errmsg += " but it already exists.";
        Assert(false, errmsg.data());
        return;
    }
    m_components.emplace(ObjectId, TComp());
}


template <typename TComp>
void ComponentPool<TComp>::StartThisPool(Scene* scene)
{
    DEBUG_PRINT_DATA_FLOW
    for (auto& i : m_components)
    {
        if (i.second.GetOwner()->GetScene() == scene && i.second.IsEnabled())
        {
            i.second.Start();
        }
    }
}
template <typename TComp>
void ComponentPool<TComp>::UpdateThisPool(Scene* scene, float dt)
{
    for (auto& i : m_components)
    {
        if (i.second.GetOwner()->GetScene() == scene &&i.second.IsEnabled())
        {
			if (i.second.GetUpdateCounter()==0)
			{
				i.second.ResetUpdateCounter();
				i.second.Update(dt);
			}
			else
			{
				i.second.DecrementUpdateCounter();
			}
        }
    }
}

template <typename TComp>
TComp& ComponentPool<TComp>::GetComponentRef(ObjectId id)
{
    for (auto& i : m_components)
    {
        if (i.first == id)
        {
            return i.second;
        }
    }
    throw ComponentPoolErrorException("Component does not exist in pool.");
}

template <typename TComp>
bool ComponentPool<TComp>::HasComponent(ObjectId id)
{
    DEBUG_PRINT_DATA_FLOW
    for (auto& i : m_components)
    {
        if (i.first == id)
        {
            return true;
        }
    }
    return false;
}

template <typename TComp>
void ComponentPool<TComp>::RemoveComponent(ObjectId id)
{
	//todo refactor this
	Assert(0, "Removing component is not currently supported.");
    DEBUG_PRINT_DATA_FLOW
    for (auto& i : m_components)
    {
        if (i.first == id)
        {
            m_components.erase(id);
            return;
        }
    }
    throw ComponentPoolErrorException("Component does not exist in pool.");
}

