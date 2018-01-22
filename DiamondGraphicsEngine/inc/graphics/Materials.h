#pragma once
#include "graphics/Color.h"

namespace Graphics
{
    enum class TextureType;
    class TextureManager;
    class ShaderProgram;
    enum class ShaderType;
    class Texture;
    class GraphicsEngine;

    class Material
    {
        Material() = default;
        /*******************************************************
        * @brief Creates a material and reads its data from file.
        * @param fileName Relative material file name
        * @note This function will not be used for assignments
        ******************************************************/
        Material(std::string const& fileName);
        /******************************************************
        * @brief Creates a material and reads its data from file.
        * @param fileName Relative material file name.
        * @note This function will not be used for assignments
        ******************************************************/
        virtual void LoadMaterial(std::string const& fileName);
        friend class MaterialManager;
    public:
        //copy is allowed
        Material(Material const& fileName)=default;

        virtual ~Material() = default;

        virtual void SetShaderParameters(std::shared_ptr<ShaderProgram> shader, Graphics::GraphicsEngine* g);
        
        virtual void Reflect(TwBar* editor, std::string const& groupName, Graphics::GraphicsEngine* g);

        //=========================================================================
        //                  Getters and Setters
        //=========================================================================

        ////////////////////////////////////////////////////////////////////////////
        /// material name
        virtual std::string GetMaterialName() const { return m_materialName; }
        virtual Material& SetMaterialName(std::string const& name);

        ////////////////////////////////////////////////////////////////////////////
        /// ambient color
        virtual Color GetAmbientColor() const { return m_ambientColor; }
        virtual Material& SetAmbientColor(Color const& color);

        ////////////////////////////////////////////////////////////////////////////
        /// diffuse color
        virtual Color GetDiffuseColor() const { return m_diffuseColor; }
        virtual Material& SetDiffuseColor(Color const& color);

        ////////////////////////////////////////////////////////////////////////////
        /// specular color
        virtual Color GetSpecularColor() const { return m_specularColor; }
        virtual Material& SetSpecularColor(Color const& color);

        ////////////////////////////////////////////////////////////////////////////
        /// emissive color
        virtual Color GetEmissiveColor() const { return m_emissiveColor; }
        virtual Material& SetEmissiverColor(Color const& color);

        ////////////////////////////////////////////////////////////////////////////
        /// specularity
        virtual f32 GetSpecularExponent() const { return m_specularExponent; }
        virtual Material& SetSpecularExponent(f32 pow);

        ////////////////////////////////////////////////////////////////////////////
        /// diffuse texture
        virtual std::shared_ptr<Texture> GetDiffuseTexture() const { return m_diffuseTexture.second; }
        virtual Material& AssignDiffuseTexture(TextureType type, std::shared_ptr<Texture> texture);
        virtual void SetDiffuseTextureEnabled(bool enabled) { m_isDiffuseTextureEnabled = enabled; }

        ////////////////////////////////////////////////////////////////////////////
        /// specular texture
        virtual std::shared_ptr<Texture> GetSpecularTexture() const { return m_specularTexture.second; }
        virtual Material& AssignSpecularTexture(TextureType type, std::shared_ptr<Texture> texture);
        virtual void SetSpecularTextureEnabled(bool enabled) { m_isSpecularTextureEnabled = enabled; }

        ////////////////////////////////////////////////////////////////////////////
        /// normal map texture
        virtual std::shared_ptr<Texture> GetNormalMapTexture() const { return m_normalMapTexture.second; }
        virtual Material& AssignNormalMapTexture(TextureType type, std::shared_ptr<Texture> texture);
        virtual void SetNormalMapTextureEnabled(bool enabled) { m_isNormalMapTextureEnabled = enabled; }

        ////////////////////////////////////////////////////////////////////////////
        /// object transparency
        virtual f32 GetOpacity() const { return m_opacity; }
        virtual Material& SetOpacity(f32 alpha);

        ////////////////////////////////////////////////////////////////////////////
        /// illumination model index
        virtual int GetIlluminationModel() const { return m_illumModel; }
        virtual Material& SetIlluminationModel(int index);

        ////////////////////////////////////////////////////////////////////////////
        /// if receive light
        virtual bool IfReceiveLight() const { return m_ifReceiveLight; }
        virtual Material& SetIfReceiveLight(bool receive);

    private:
        std::string m_materialName;

        bool m_ifReceiveLight = true;

        bool m_isDiffuseTextureEnabled = true;
        bool m_isSpecularTextureEnabled = false;
        bool m_isNormalMapTextureEnabled = false;


        //TODO: Change the following pairs to a TextureHandle
        ///textures needed for an object
        std::pair<TextureType, std::shared_ptr<Texture> >m_diffuseTexture;
        std::pair<TextureType, std::shared_ptr<Texture> >m_specularTexture;
        std::pair<TextureType, std::shared_ptr<Texture> >m_normalMapTexture;

        Color m_ambientColor;
        Color m_diffuseColor = Color(1, 1, 1);
        Color m_specularColor;
        Color m_emissiveColor;

        f32 m_specularExponent = 1;
        f32 m_opacity = 1;//not used but nice to have for the future.
        //mtl file illum index frmo 0 to 10, as in wavefront mtl
        int m_illumModel = 2;

        

    private://reserved for editor
        
        //todo make the structure better
        size_t m_dTextureEnumUIntVal = 0;
        size_t m_sTextureEnumUIntVal = 0;
        size_t m_nTextureEnumUIntVal = 0;
        struct EditorWrapper
        {
            static std::vector<std::shared_ptr<Graphics::Texture> > textures;
            static void TW_CALL SetDiffuseTextureCB(const void* value, void* clientData);
            static void TW_CALL GetDiffuseTextureCB(void* value, void* clientData);
            static void TW_CALL SetSpecularTextureCB(const void* value, void* clientData);
            static void TW_CALL GetSpecularTextureCB(void* value, void* clientData);
            static void TW_CALL SetNormalmapTextureCB(const void* value, void* clientData);
            static void TW_CALL GetNormalmapTextureCB(void* value, void* clientData);

        };



    };
}
