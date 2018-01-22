#include "Precompiled.h"
#include "graphics/LightManager.h"
#include "graphics/ShaderProgram.h"

namespace Graphics
{
    std::list<LightAttribute> LightManager::m_lightAttribtues;


    void LightAttribute::SetLightUniform(int index ,std::shared_ptr<ShaderProgram> shader) const
    {
		//TODO(STUDENT): Set light attributes needed to render.
        std::stringstream str;
        str << "Lights[" << index << "].";
        //colors
        shader->SetUniform(str.str() + "ambient", ambientColor);
        shader->SetUniform(str.str() + "diffuse", diffuseColor);
        shader->SetUniform(str.str() + "specular", specularColor);
        //other attribute
        shader->SetUniform(str.str() + "isActive",isActive);
        shader->SetUniform(str.str() + "direction", direction);
        shader->SetUniform(str.str() + "type", static_cast<int>(type));
        shader->SetUniform(str.str() + "intensity", intensity);
        if (ifDecay)
        {
            shader->SetUniform(str.str() + "distanceAttenuation", disAtten);
        }        
        switch (type)
        {
        case LightType::Directional:
            break;
        case LightType::Point:
            shader->SetUniform(str.str() + "position", position);
            break;
        case LightType::Spot:
            shader->SetUniform(str.str() + "position", position);
            //TODO(STUDENT): Set innerAngle
            //TODO(STUDENT): Set outerAngle
            //TODO(STUDENT): Set spotlightFallOff
            break;
        default:
            break;
        }

    }

    LightBase::LightBase()
    {
        m_attribute = LightManager::GetNewLightAttribute();
    }

    LightBase::~LightBase()
    {
        if (m_isCopied)
        {
            //BUG memory leak
            //LightManager::DeleteLightAttribute(m_attribute);
        }
    }

    LightBase::LightBase(LightBase const& rhs)
    {
        m_attribute = rhs.m_attribute;
        m_isCopied = true;
    }

    LightBase& LightBase::operator=(LightBase const& rhs)
    {
        m_attribute = rhs.m_attribute;
        m_isCopied = true;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////
    //  Getters
    ////////////////////////////////////////////////////////////////////
    LightType LightBase::GetLightType() const
    {
        return m_attribute->type;
    }

    float LightBase::GetIntensity() const
    {
        return m_attribute->intensity;
    }

    Math::Vector3 LightBase::GetDirection() const
    {
        return { m_attribute->direction.x,m_attribute->direction.y,m_attribute->direction.z };
    }

    float LightBase::GetInnerAngle() const
    {
        return m_attribute->innerAngle;
    }

    float LightBase::GetOuterAngle() const
    {
        return m_attribute->outerAngle;
    }

    float LightBase::GetSpotlightFalloff() const
    {
        return m_attribute->spotFalloff;
    }

    bool LightBase::GetIfDecay() const
    {
        return m_attribute->ifDecay;
    }

    Math::Vector3 LightBase::GetDistanceAttenuation() const
    {
        return m_attribute->disAtten;
    }

    Color LightBase::GetDiffuseColor() const
    {
        return m_attribute->diffuseColor;
    }

    Color LightBase::GetAmbientColor() const
    {
        return m_attribute->ambientColor;
    }

    Color LightBase::GetSpecularColor() const
    {
        return m_attribute->specularColor;
    }
    ////////////////////////////////////////////////////////////////////
    //  Setters
    ////////////////////////////////////////////////////////////////////

    LightBase* LightBase::SetLightType(LightType type)
    {
        m_attribute->type = type;
        return this;
    }

    LightBase* LightBase::SetIntensity(float intensity)
    {
        m_attribute->intensity = intensity;
        return this;
    }

    LightBase* LightBase::SetDirection(Math::Vector3 const& dir)
    {
        m_attribute->direction.x = dir.x;
        m_attribute->direction.y = dir.y;
        m_attribute->direction.z = dir.z;
        return this;
    }

    LightBase* LightBase::SetInnerAngle(float radians)
    {
        m_attribute->innerAngle = radians;
        return this;
    }

    LightBase* LightBase::SetOuterAngle(float radians)
    {
        m_attribute->outerAngle = radians;
        return this;
    }

    LightBase* LightBase::SetSpotlightFalloff(float val)
    {
        m_attribute->spotFalloff = val;
        return this;
    }

    LightBase* LightBase::SetIfDecay(bool ifDecay)
    {
        m_attribute->ifDecay = ifDecay;
        return this;
    }

    LightBase* LightBase::SetDistanceAttenuation(Math::Vector3 const& distAtten)
    {
        m_attribute->disAtten = distAtten;
        return this;
    }

    LightBase* LightBase::SetDistanceAttenuation(float constant, float linear, float quadratic)
    {
        m_attribute->disAtten = { constant ,linear ,quadratic };
        return this;
    }

    LightBase* LightBase::SetDiffuseColor(Color const& diffuseColor)
    {
        m_attribute->diffuseColor = diffuseColor;
        return this;
    }

    LightBase* LightBase::SetAmbientColor(Color const& ambientColor)
    {
        m_attribute->ambientColor = ambientColor;
        return this;
    }

    LightBase* LightBase::SetSpecularColor(Color const& specularColor)
    {
        m_attribute->specularColor = specularColor;
        return this;
    }

    LightAttributeHandle LightManager::GetNewLightAttribute()
    {
        m_lightAttribtues.emplace_front(LightAttribute());
        return m_lightAttribtues.begin();
    }

    void LightManager::DeleteLightAttribute(LightAttributeHandle attr)
    {
        m_lightAttribtues.erase(attr);
    }

    void LightManager::SetLightsUniform(std::shared_ptr<ShaderProgram> shader)
    {
        shader->SetUniform("LightCount", static_cast<int>(m_lightAttribtues.size()));
        int index = 0;
        for (auto& i : m_lightAttribtues)
        {
            i.SetLightUniform(index++, shader);
        }
    }
}

