#pragma once

#include "core/Object.h"
#include "core/ComponentPool.h"

//flag for if a component needs acceess to graphics engine
#define SHADED true
//flag for if a component needs acceess to graphics engine
#define UNSHADED false

namespace Graphics {
    enum class ShaderType;
    class ShaderProgram;
    class GraphicsEngine;
}

/***********************************************************
* @brief exception class used for component errors
***********************************************************/
class ComponentErrorException
    : public std::exception
{
    std::string m_msg;
public:
    explicit ComponentErrorException(std::string  msg) : m_msg(std::move(msg)) {}
    char const* what() const override { return m_msg.c_str(); }
};

/*******************************************************
 * @brief This is a component editor interface. ComponenetBase
 * is derived from this class so that each component can access
 * and have its own place in the component editor.
 * For components that need to appear in the editor, their name must be
 * registered by calling REGISTER_EDITOR_COMPONENT(ClassType)
 *******************************************************/
class ComponentEditorInterface
{
public:
    virtual ~ComponentEditorInterface() = 0{}

    /*******************************************************
    * @brief Get component name, must be registered. Used by editor.
    * @return Registered component name.
    *******************************************************/
    virtual std::string GetComponentTypeName() { Warning("Calling base function to get component name since component is not registered."); return { "Unknown Component" }; }
    /*******************************************************
    * @brief Implement this function if you wish the component
    * appear in Component Editor.
    *******************************************************/
    virtual void Reflect(TwBar*, std::string const& /*barName*/, std::string const& /*groupName*/, Graphics::GraphicsEngine*) {}
};
/***********************************************************
 * @brief Common component interface. The very base level
 * of a component inheritance structure.
 * @remark All templated components can be cast to this type.
 **********************************************************/
class ComponentInterface
{
public:
    virtual ~ComponentInterface() = 0 {}
    virtual void Start() {}
    /*******************************************************
     * @brief Update function will be called each frame as long as
     * the component is enabled.
     * @param dt frametime in seconds
     *******************************************************/
    virtual void Update(float dt) { UNUSED_VAR(dt) }
    /*******************************************************
     * @brief Set whatever needed to render this object
     * @remark This function will ONLY be called when ifShaded flag is set
     *******************************************************/
    virtual void SetShaderParams(std::shared_ptr<Graphics::ShaderProgram>, Graphics::GraphicsEngine*) {}
    /*******************************************************
     * @brief Enable current component, call event callback
     *******************************************************/
    virtual void Enable() { OnEnable(); }
    /*******************************************************
     * @brief Disable current component, call event callback
     *******************************************************/
    virtual void Disable() { OnDisable(); }
    /*******************************************************
     * @brief Set if current component is enabled, 
     * @remark will NOT call any callback functions
     *******************************************************/
    virtual void SetEnabled(bool){}
    virtual bool IsEnabled() { return false; }

protected:
    virtual void OnEnable() {}
    virtual void OnDisable() {}
};

/**************************************************************************
* @brief Component class base. Creates component pool automatically
* @tparam TComp Type of derived component
* @tparam ifShaded Specify if this component needs access to rendering engine.
* If true, virtual function SetShaderParams will be called each frame.
* @note Be aware of shader efficiency, some uniforms only need to be set once 
* per frame, such as Light attribute, camera view vector etc.
* If it is shaded, SetShaderParams will be called each frame on every object.
*************************************************************************/
template <typename TComp, bool ifShaded>
class ComponentBase
    : public ComponentInterface
    , public ComponentEditorInterface
{
    friend class Object;
public:
    explicit ComponentBase(bool defaultEnable, Graphics::ShaderType shaderType = Graphics::ShaderType::Null) : m_isEnabled(defaultEnable), m_shaderType(shaderType){}
    virtual ~ComponentBase() = 0 {}

    void Enable() override;
    void Disable() override;

	bool IsEnabled() override { return m_isEnabled; }

    void SetEnabled(bool ifEnabled) override { m_isEnabled = ifEnabled; }

	Object* GetOwner() const { return m_owner; }

    static constexpr bool IfShaded() { return ifShaded; }
    const Graphics::ShaderType& GetShaderType() const { return m_shaderType; }

    /*******************************************************
	 * @brief Get update "frequency".
	 * @return How many frames to update once.
	 *******************************************************/
	unsigned GetUpdateGap() const { return m_updateFrameGap; }
    /*******************************************************
	 * @brief Set update "frequency".
	 * @param value How many frames to update once.
	 *******************************************************/
	void SetUpdateGap(unsigned value) { m_updateFrameGap = value; }
    /*******************************************************
	 * @brief This get the counter to update. Whenever the counter reaches
	 *  zero, update function will be called.
	 * @return How many frames left to update.
	 *******************************************************/
	unsigned GetUpdateCounter()const { return m_updateCounter; }
    /*******************************************************
	 * @brief Reset the counter to be the update "frequency" defined in the class
	 *******************************************************/
	virtual void ResetUpdateCounter() { m_updateCounter = m_updateFrameGap; }
    /*******************************************************
	 * @brief Decrement the counter number.
	 * Whenever the counter reaches zero, update function will be called.
	 *******************************************************/
	virtual void DecrementUpdateCounter() { if (m_updateCounter > 0)--m_updateCounter; }


protected:
    Object* m_owner = nullptr;
    bool m_isEnabled = true;

    Graphics::ShaderType m_shaderType = Graphics::ShaderType::Null;

    //how many frame to update once. 
	//i.e. if enabled,
	//0 means to update this component each frame,
	//1 means to update this component every other frame
	//2 means to update this component once per three frames
    unsigned m_updateFrameGap = 0;
	//update counter will be decreased by Update() by one whenver it gets called.
	//if the value reaches 0, it will be reset to m_updateFrameGap
	unsigned m_updateCounter = m_updateFrameGap;
private:

    /*******************************************************
     * @brief This is the most interesting part of the architecture.
     * The compiler will generate a container specifically for this
     * component type. 
     *******************************************************/
    static ComponentPool<TComp> m_componentPool;

    void AssignOwner(Object* owner);
};
//=============================================================================
///////////////////////////////////////////////////////////////////////////////
//                              Implementation
///////////////////////////////////////////////////////////////////////////////
//=============================================================================

template <typename TComp, bool ifShaded>
void ComponentBase<TComp, ifShaded>::Enable()
{
    DEBUG_PRINT_DATA_FLOW
    m_isEnabled = true;
    OnEnable();
}

template <typename TComp, bool ifShaded>
void ComponentBase<TComp, ifShaded>::Disable()
{
    DEBUG_PRINT_DATA_FLOW
    m_isEnabled = false;
    OnDisable();
}

template <typename TComp, bool ifShaded>
void ComponentBase<TComp, ifShaded>::AssignOwner(Object* owner)
{
    Assert(owner!=nullptr, "Trying to assign an object to null.");
    DEBUG_PRINT_DATA_FLOW
    m_owner = owner;
    if (ifShaded)
    {
        m_owner->PushShadedComponent(this, m_shaderType);
    }
}
