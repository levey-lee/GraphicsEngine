#include "Precompiled.h"
#include "core/components/Light.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/LightManager.h"
#include "core/components/Transform.h"
#include "core/TwImpl.h"


namespace
{
    TwBar* editorCache;
    std::string barNameCache;
    std::string groupNameCache;
    Graphics::GraphicsEngine* graphicsCache;
}
void SetLightEditor(Component::Light* light);

namespace TwCallBack
{
    void TW_CALL GetLightEnable(void *value, void *clientData)
    {
        Component::Light* light = static_cast<Component::Light*>(clientData);
        *static_cast<bool*>(value) = light->IsEnabled();
    }
    void TW_CALL SetLightEnable(const void *value, void *clientData)
    {
        Component::Light* light = static_cast<Component::Light*>(clientData);
        bool enabled = *static_cast<const bool*>(value);
        if (enabled)
        {
            light->Enable();
        }
        else
        {
            light->Disable();
        }
    }

    void TW_CALL GetConeAngle(void *value, void *clientData)
    {
        float angleRad = *static_cast<float*>(clientData);
        float angleDeg = Math::RadToDeg(angleRad);
        *static_cast<float*>(value) = angleDeg;
    }
    void TW_CALL SetConeAngle(const void *value, void *clientData)
    {
        float angleRad = *static_cast<const float*>(value);
        float angleDeg = Math::DegToRad(angleRad);
       *static_cast<float*>(clientData) = angleDeg;
    }


    void TW_CALL GetLightType(void* value, void* clientData)
    {
        Component::Light* light = static_cast<Component::Light*>(clientData);
        *static_cast<int*>(value) = static_cast<int>(light->GetLightType());
        
    }
    void TW_CALL SetLightType(const void* value, void* clientData)
    {
        Component::Light* light = static_cast<Component::Light*>(clientData);
        light->SetLightType(static_cast<Graphics::LightType>(*static_cast<const int*>(value)));
        //bug SetLightEditor(light);
    }

    
}
void SetLightEditor(Component::Light* light)
{
    //bug TwRemoveVar(editorCache, groupNameCache.c_str());
    std::string defStr = "group=" + groupNameCache;
    Graphics::LightType lightType = light->GetLightType();
    TwAddVarCB(editorCache, nullptr, TW_TYPE_BOOLCPP, TwCallBack::SetLightEnable, TwCallBack::GetLightEnable, light, (defStr + " label=Enabled").c_str());
    TwType lightTypeEnumType = TwDefineEnumFromString(nullptr, "Directional,Point,Spot");
    TwAddVarCB(editorCache, nullptr, lightTypeEnumType, TwCallBack::SetLightType, TwCallBack::GetLightType, light, (defStr + " label='Light Type'").c_str());
    TwAddVarRW(editorCache, nullptr, TW_TYPE_FLOAT, &light->GetLightAttribute()->intensity, (defStr + " label=Intensity step=0.01 min=0").c_str());
    TwAddVarRW(editorCache, nullptr, TW_TYPE_FLOAT, &light->GetLightAttribute()->range, (defStr + " label=Range step=0.01 min=0").c_str());

    TwType shadowTypeEnumType = TwDefineEnumFromString(nullptr, "No Shadow,Hard Shadow,Soft Shadow");
    TwAddVarRW(editorCache, nullptr, shadowTypeEnumType, &light->GetLightAttribute()->shadowType, (defStr + " label='Shadow Type'").c_str());
    TwAddVarRW(editorCache, nullptr, TW_TYPE_FLOAT, &light->GetLightAttribute()->shadowStrength, (defStr + " label='Shadow Strength'").c_str());

    //bug if (lightType == Graphics::LightType::Directional)
    {
        TwAddVarRO(editorCache, nullptr, TW_TYPE_DIR3F, &light->GetLightAttribute()->direction, (defStr + " label=Direction").c_str());
    }
    TwAddVarRW(editorCache, nullptr, TW_TYPE_COLOR3F, &light->GetLightAttribute()->ambientColor, (defStr + " label=Ambient").c_str());
    TwAddVarRW(editorCache, nullptr, TW_TYPE_COLOR3F, &light->GetLightAttribute()->diffuseColor, (defStr + " label=Diffuse").c_str());
    TwAddVarRW(editorCache, nullptr, TW_TYPE_COLOR3F, &light->GetLightAttribute()->specularColor, (defStr + " label=Specular").c_str());
    //bug if (lightType == Graphics::LightType::Spot)
    {
        TwAddVarCB(editorCache, nullptr, TW_TYPE_FLOAT, TwCallBack::SetConeAngle, TwCallBack::GetConeAngle, &light->GetLightAttribute()->innerAngle, (defStr + " label='Inner Cone Angle' min=0 max=180").c_str());
        TwAddVarCB(editorCache, nullptr, TW_TYPE_FLOAT, TwCallBack::SetConeAngle, TwCallBack::GetConeAngle, &light->GetLightAttribute()->outerAngle, (defStr + " label='Outer Cone Angle' min=0 max=180").c_str());
        TwAddVarRW(editorCache, nullptr, TW_TYPE_FLOAT, &light->GetLightAttribute()->spotFalloff, (defStr + " label='Spotlight Falloff Value' step=0.01").c_str());
    }
    TwAddVarRW(editorCache, nullptr, TW_TYPE_BOOLCPP, &light->GetLightAttribute()->ifDecay, (defStr + " label=Decay").c_str());
    TwAddVarRW(editorCache, nullptr, TW_TYPE_POINT(3, 0.01f, "Constant", "Linear", "Quadratic"), &light->GetLightAttribute()->disAtten, (defStr + " label='Distance Attenuation'").c_str());
}

namespace Component
{
    void Light::Start()
    {
        m_attribute->isActive = m_isEnabled;
    }

    void Light::Update(float)
    {
        auto const& temPos = m_owner->GetComponentRef<Transform>().GetPosition();
        m_attribute->position.x = temPos.x;
        m_attribute->position.y = temPos.y;
        m_attribute->position.z = temPos.z;


        if (m_owner)
        {
            auto& trans = m_owner->GetComponentRef<Transform>();
            Math::Matrix4 const& worldTrans = trans.GetWorldTransform();
            Math::Vector3 newDir = Math::TransformNormal(worldTrans, Math::Vector3(0, 0, -1));
            m_attribute->direction = Math::Vector4(newDir.x, newDir.y, newDir.z, 0);

            //viewproj = projection * view
            Math::Matrix4 projection;
            projection.ZeroOut();
            if (m_attribute->lightType == Graphics::LightType::Directional)
            {//todo
                projection = projection.CreateOrthographic(1000, 1000, 1, 1000);
            }
            else if (m_attribute->lightType == Graphics::LightType::Point)
            {
                //TODO add perspective
            }
            if (m_attribute->shadowType == Graphics::ShadowType::HardShadow)
            {
                m_attribute->viewproj = projection*worldTrans.Inverted();
            }
        }
    }

    void Light::Reflect(TwBar* editor, std::string const& barName, std::string const& groupName,
        Graphics::GraphicsEngine* graphics)
    {
        editorCache = editor;
        barNameCache = barName;
        groupNameCache = groupName;
        graphicsCache = graphics;
        std::string defStr = "group=" + groupName;

        SetLightEditor(this);
    }


    void Light::OnEnable()
    {
        m_attribute->isActive = true;
    }

    void Light::OnDisable()
    {
        m_attribute->isActive = false;
    }
}

