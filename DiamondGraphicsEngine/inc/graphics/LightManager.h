#pragma once
#include "math/Vector3.h"
#include "graphics/Color.h"
#include "math/Vector4.h"
#include "math/Matrix4.h"

namespace Graphics
{
    struct LightAttribute;
    class ShaderProgram;
    class LightBase;
    class LightManager;
    using LightAttributeHandle = std::list<LightAttribute>::iterator;

    enum class LightType
    {
        Directional,
        Point,
        Spot
        //Area
        //etc...
    };
    enum class ShadowType
    {
        NoShadow,
        HardShadow,
        SoftShadow
    };

    struct LightAttribute
    {
        //this will be written by light component
        bool isActive = true;
        //this will be written by light component
        Math::Vector4 position = Math::Vector4( 0,0,0,1 );
        Math::Vector4 direction = Math::Vector4(0, 0, -1, 0);
        LightType lightType = LightType::Directional;
        ShadowType shadowType = ShadowType::NoShadow;
        float shadowStrength = 1.0f;
        float shadowFov = 0.45f;
        int filterWidth = 8;
        float intensity = 1.0f;
        float range = 10.0f;
        float nearPlane = 1;
        float farPlane = 80;
        float shadowExp = 150.0f;
        float innerAngle = Math::c_Pi / 12.0f;//15 degree
        float outerAngle = Math::c_Pi / 6.0f;//30 degree
        float spotFalloff = 1.0f;
        //distance attenuation will only be set when this is true.
        bool ifDecay = true;
        Math::Vector3 disAtten = { 0.1f,0.1f,0.1f };//constant, linear, quadratic
        Color diffuseColor = Color(1, 1, 1);
        Color ambientColor =  Color(0, 0, 0);
        Color specularColor = Color(0, 0, 0);
        Math::Matrix4 viewproj;
        void SetLightUniform(int index ,std::shared_ptr<ShaderProgram> program) const;
    };
    
    class LightBase
    {
    public:
        LightBase();
        virtual ~LightBase();
        LightBase(LightBase const& rhs);

        LightBase& operator=(LightBase const& rhs);
        ////////////////////////////////////////////////////////////////////////
        //          Getters and Setters
        ////////////////////////////////////////////////////////////////////////
        virtual LightType GetLightType() const;
        virtual float GetIntensity() const;
        virtual Math::Vector3 GetDirection() const;
        virtual float GetInnerAngle() const;
        virtual float GetOuterAngle() const;
        virtual float GetSpotlightFalloff() const;
        virtual bool GetIfDecay() const;
        virtual Math::Vector3 GetDistanceAttenuation() const;
        virtual Color GetDiffuseColor() const;
        virtual Color GetAmbientColor() const;
        virtual Color GetSpecularColor() const;
        virtual ShadowType GetShadowType() const;
        ////////////////////////////////////////////////////////////////////////
        virtual LightBase* SetLightType(LightType type);
        virtual LightBase* SetIntensity(float intensity);
        virtual LightBase* SetInnerAngle(float radians);
        virtual LightBase* SetOuterAngle(float radians);
        virtual LightBase* SetSpotlightFalloff(float val);
        virtual LightBase* SetIfDecay(bool ifDecay);
        virtual LightBase* SetDistanceAttenuation(Math::Vector3 const& distAtten);
        virtual LightBase* SetDistanceAttenuation(float constant, float linear, float quadratic);
        virtual LightBase* SetDiffuseColor(Color const& diffuseColor);
        virtual LightBase* SetAmbientColor(Color const& ambientColor);
        virtual LightBase* SetSpecularColor(Color const& specularColor);
        virtual LightBase* SetShadowType(ShadowType type);
        LightAttributeHandle GetLightAttribute() const { return m_attribute; }
    protected:
        LightAttributeHandle m_attribute;
    private:
        bool m_isCopied = false;
    };
    
    class LightManager
    {
    public:
        static LightAttributeHandle GetNewLightAttribute();
        static void DeleteLightAttribute(LightAttributeHandle attr);
        void SetLightsUniform(std::shared_ptr<ShaderProgram> shader);
        void SetLightShadowUniforms(std::shared_ptr<ShaderProgram> shader);
        void SetShadowFilterUniforms(std::shared_ptr<ShaderProgram> shader);
        //todo return a list of matrix
        Math::Matrix4 GetLightViewProj();
        Math::Vec3 GetShadowingLightPos();
    private:
        static std::list<LightAttribute> m_lightAttribtues;
        float GaussianWeights[101/*MUST MATCH with shader width*2+1 */] = { 0 };
    };

}
