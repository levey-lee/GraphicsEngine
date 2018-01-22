#include "Precompiled.h"
#include "graphics/Materials.h"
#include "framework/Debug.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/ShaderProgram.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"

std::vector<std::shared_ptr<Graphics::Texture> > Graphics::Material::EditorWrapper::textures;

namespace Graphics
{
    Material::Material(std::string const& fileName)
    {
        Material::LoadMaterial(fileName);
    }

    void Material::LoadMaterial(std::string const& fileName)
    {//TODO: load one material from *.mtl, this file should only contain 1 material
    }

    void Material::SetShaderParameters(std::shared_ptr<ShaderProgram> shader, Graphics::GraphicsEngine* g)
    {
        std::shared_ptr<ShaderManager> shaderManager = g->GetShaderManager();
        
        shader->SetUniform("Material.ReceiveLight", m_ifReceiveLight);
        
        /// numbers from 0 to 10 as mtl file format illum model index
        /// see reference at https://en.wikipedia.org/wiki/Wavefront_.obj_file
        /// see datailed refernce at http://paulbourke.net/dataformats/mtl/
        switch (m_illumModel)
        {
        case 0://Color on and Ambient off
        //TODO(Assignment 1): Set material uniform to shader, none should be hardcoded.
#if SAMPLE_IMPLEMENTATION
            shader->SetUniform("Material.DiffuseColor", m_diffuseColor);
            shader->SetUniform("Material.EmissiveColor", m_emissiveColor);
#endif // 0

            break;
        case 1://Color on and Ambient on
            shader->SetUniform("Material.AmbientColor", m_ambientColor);
#if SAMPLE_IMPLEMENTATION
            //TODO(Assignment 1): Set material uniform to shader, none should be hardcoded.
            shader->SetUniform("Material.DiffuseColor", m_diffuseColor);
            shader->SetUniform("Material.EmissiveColor", m_emissiveColor);
#endif // SAMPLE_IMPLEMENTATION

            break;
        case 2://Specular Highlight on
        //TODO(Assignment 2): Set material uniform to shader, none should be hardcoded.
#if SAMPLE_IMPLEMENTATION
            shader->SetUniform("Material.DiffuseColor", m_diffuseColor);
            shader->SetUniform("Material.AmbientColor", m_ambientColor);
            shader->SetUniform("Material.EmissiveColor", m_emissiveColor);
            shader->SetUniform("Material.SpecularColor", m_specularColor);
            shader->SetUniform("Material.SpecularExponent", m_specularExponent);
#endif // SAMPLE_IMPLEMENTATION

            break;
        case 3://Reflection on and Ray trace on
            break;
        case 4://Transparency: Glass on, Reflection: Ray trace on
            break;
        case 5://Reflection: Fresnel on and Ray trace on
            break;
        case 6://Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
            break;
        case 7://Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
            break;
        case 8://Reflection on and Ray trace off
            break;
        case 9://Transparency: Glass on, Reflection: Ray trace off
            break;
        case 10://Casts shadows onto invisible surfaces
            break;
        default:
            Error("Unknown material illumination model index.");
            break;
        }

        //TODO(Assignment 2): Set material textures uniform to shader, none should be hardcoded.
        //You must also check nullptrs and if they are enabled before sending data to GPU
        //To set texture, you call a function - TextureManager::BindTexture();
#if SAMPLE_IMPLEMENTATION
        std::shared_ptr<TextureManager> textureManager = g->GetTextureManager();

        ////////////////////////////////////////////////////////////////////////
        //          Diffuse Texture
        ////////////////////////////////////////////////////////////////////////
        if (m_isDiffuseTextureEnabled && m_diffuseTexture.second != nullptr)
        {
            Assert(m_diffuseTexture.second->IsBuilt(), "Texture with name \"%s\" is not build.", m_diffuseTexture.second->GetTextureName().c_str());
            textureManager->BindTexture(m_diffuseTexture.second, shader, "Material.DiffuseTexture", m_diffuseTexture.first);
            shader->SetUniform("Material.DiffuseTextureEnabled", true);
        }
        else
        {
            shader->SetUniform("Material.DiffuseTextureEnabled", false);
        }
        ////////////////////////////////////////////////////////////////////////
        //          Specular Texture
        ////////////////////////////////////////////////////////////////////////
        if (m_isSpecularTextureEnabled && m_specularTexture.second != nullptr)
        {
            Assert(m_specularTexture.second->IsBuilt(), "Texture with name \"%s\" is not build.", m_specularTexture.second->GetTextureName().c_str());
            textureManager->BindTexture(m_specularTexture.second, shader, "Material.SpecularTexture", m_specularTexture.first);
            shader->SetUniform("Material.SpecularTextureEnabled", true);
        }
        else
        {
            shader->SetUniform("Material.SpecularTextureEnabled", false);
        }
        ////////////////////////////////////////////////////////////////////////
        //          Normal Map Texture
        ////////////////////////////////////////////////////////////////////////
        if (m_isNormalMapTextureEnabled && m_normalMapTexture.second != nullptr)
        {
            Assert(m_normalMapTexture.second->IsBuilt(), "Texture with name \"%s\" is not build.", m_normalMapTexture.second->GetTextureName().c_str());
            textureManager->BindTexture(m_normalMapTexture.second, shader, "Material.NormalMapTexture", m_normalMapTexture.first);
            shader->SetUniform("Material.NormalMapTextureEnabled", true);
        }
        else
        {
            shader->SetUniform("Material.NormalMapTextureEnabled", false);
        }
#endif // SAMPLE_IMPLEMENTATION

    }

    void Material::Reflect(TwBar* editor, std::string const& groupName, GraphicsEngine* g)
    {
        std::string defStr = "group='" + groupName + "'";
        TwAddVarRO(editor, nullptr, TW_TYPE_STDSTRING, &m_materialName, (defStr+ " label='Material Name'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_BOOLCPP, &m_ifReceiveLight, (defStr + " label='Receive Light'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_FLOAT, &m_opacity, (defStr + " label='Opacity' step=0.01 min=0 max=1").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_COLOR3F, &m_ambientColor, (defStr+ " label='Ambient Color'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_COLOR3F, &m_diffuseColor, (defStr+ " label='Diffuse Color'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_COLOR3F, &m_specularColor, (defStr+ " label='Specular Color'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_COLOR3F, &m_emissiveColor, (defStr+ " label='Emissive Color'").c_str());
        TwAddVarRW(editor, nullptr, TW_TYPE_FLOAT, &m_specularExponent, (defStr+ " label='Speuclar Exponent' step=0.01").c_str());        
        {//Illumination model enum
            std::string enumStr;
            enumStr = enumStr + "[0] " + "Color on and Ambient off,";
            enumStr = enumStr + "[1] " + "Color on and Ambient on,";
            enumStr = enumStr + "[2] " + "Specular Highlight on,";
            enumStr = enumStr + "[3] " + "Reflection on and Ray trace on,";
            enumStr = enumStr + "[4] " + "Transparency: Glass on; Reflection: Ray trace on,";
            enumStr = enumStr + "[5] " + "Reflection: Fresnel on and Ray trace on,";
            enumStr = enumStr + "[6] " + "Transparency: Refraction on; Reflection: Fresnel off and Ray trace on,";
            enumStr = enumStr + "[7] " + "Transparency: Refraction on; Reflection: Fresnel on and Ray trace on,";
            enumStr = enumStr + "[8] " + "Reflection on and Ray trace off,";
            enumStr = enumStr + "[9] " + "Transparency: Glass on; Reflection: Ray trace off,";
            enumStr = enumStr + "[10] " + "Casts shadows onto invisible surfaces,";
            TwType illumModelType = TwDefineEnumFromString(nullptr, enumStr.c_str());
            TwAddVarRW(editor, nullptr, illumModelType, &m_illumModel, (defStr + " label='Illumination Model' ").c_str());
        }

        {//Texture enum

            std::shared_ptr<TextureManager> textureManager = g->GetTextureManager();
            EditorWrapper::textures = textureManager->GetAllTextures();
            std::string textureNames = "None,";
            if (EditorWrapper::textures.empty() == false)
            {
                for (size_t i= 0; i<EditorWrapper::textures.size();++i)
                {
                    textureNames += EditorWrapper::textures[i]->GetTextureName();
                    textureNames += ",";
                    if (EditorWrapper::textures[i] == m_diffuseTexture.second)
                    {
                        m_dTextureEnumUIntVal = i + 1;//+1 since 0 is reserved for 'None'
                    }
                    if (EditorWrapper::textures[i] == m_specularTexture.second)
                    {
                        m_sTextureEnumUIntVal = i + 1;//+1 since 0 is reserved for 'None'
                    }
                    if (EditorWrapper::textures[i] == m_normalMapTexture.second)
                    {
                        m_nTextureEnumUIntVal = i + 1;//+1 since 0 is reserved for 'None'
                    }
                }
            }
            TwAddSeparator(editor, nullptr, defStr.c_str());
            TwType textureNameTypes = TwDefineEnumFromString(nullptr, textureNames.c_str());
            TwAddVarRW(editor, nullptr, TW_TYPE_BOOLCPP, &m_isDiffuseTextureEnabled, (defStr + " label='Diffuse Texture Enabled'").c_str());
            TwAddVarCB(editor, nullptr, textureNameTypes, EditorWrapper::SetDiffuseTextureCB, EditorWrapper::GetDiffuseTextureCB, this, (defStr + " label='Diffuse Texture'").c_str());

            TwAddSeparator(editor, nullptr, defStr.c_str());
            TwAddVarRW(editor, nullptr, TW_TYPE_BOOLCPP, &m_isSpecularTextureEnabled, (defStr + " label='Specular Texture Enabled'").c_str());
            TwAddVarCB(editor, nullptr, textureNameTypes, EditorWrapper::SetSpecularTextureCB, EditorWrapper::GetSpecularTextureCB, this, (defStr + " label='Specular Texture'").c_str());

            TwAddSeparator(editor, nullptr, defStr.c_str());
            TwAddVarRW(editor, nullptr, TW_TYPE_BOOLCPP, &m_isNormalMapTextureEnabled, (defStr + " label='Normal Map Enabled'").c_str());
            TwAddVarCB(editor, nullptr, textureNameTypes, EditorWrapper::SetNormalmapTextureCB, EditorWrapper::GetNormalmapTextureCB, this, (defStr + " label='Normal Map Texture'").c_str());
        }

    }

    Material& Material::AssignDiffuseTexture(TextureType type, std::shared_ptr<Texture> texture)
    {
        m_diffuseTexture.first = type;
        m_diffuseTexture.second = texture;
        return *this;
    }

    Material& Material::AssignNormalMapTexture(TextureType type, std::shared_ptr<Texture> texture)
    {
        m_normalMapTexture.first = type;
        m_normalMapTexture.second = texture;
        return *this;
    }

    Material& Material::SetOpacity(f32 alpha)
    {
        m_opacity = alpha;
        return *this;
    }

    Material& Material::SetIlluminationModel(int index)
    {
        m_illumModel = index;
        return *this;
    }

    Material& Material::SetIfReceiveLight(bool receive)
    {
        m_ifReceiveLight = receive;
        return *this;
    }

    Material& Material::AssignSpecularTexture(TextureType type, std::shared_ptr<Texture> texture)
    {
        m_normalMapTexture.first = type;
        m_specularTexture.second = texture;
        return *this;
    }

    Material& Material::SetMaterialName(std::string const& name)
    {
        m_materialName = name;
        return *this;
    }

    Material& Material::SetAmbientColor(Color const& color)
    {
        m_ambientColor = color;
        return *this;
    }

    Material& Material::SetDiffuseColor(Color const& color)
    {
        m_diffuseColor = color;
        return *this;
    }

    Material& Material::SetSpecularColor(Color const& color)
    {
        m_specularColor = color;
        return *this;
    }

    Material& Material::SetEmissiverColor(Color const& color)
    {
        m_emissiveColor = color;
        return *this;
    }

    Material& Material::SetSpecularExponent(f32 pow)
    {
        m_specularExponent = pow;
        return *this;
    }



    void Material::EditorWrapper::SetDiffuseTextureCB(const void* value, void* clientData)
    {
        size_t textureIndex = *static_cast<const size_t *>(value);
        Material * mat = static_cast<Material *>(clientData);
        mat->m_dTextureEnumUIntVal = textureIndex;
        if (textureIndex)
        {
            mat->m_diffuseTexture.first = TextureType::DiffuseTexture_0;
            mat->m_diffuseTexture.second = textures[textureIndex - 1];//-1 since 0 is reserved for 'None', but 'None' is not in the vector
        }
        else
        {
            mat->m_diffuseTexture.second.reset();
        }
    }

    void Material::EditorWrapper::GetDiffuseTextureCB(void* value, void* clientData)
    {
        size_t textureIndex = static_cast<Material *>(clientData)->m_dTextureEnumUIntVal;
        *static_cast<size_t *>(value) = textureIndex;
    }

    void Material::EditorWrapper::SetSpecularTextureCB(const void* value, void* clientData)
    {
        size_t textureIndex = *static_cast<const size_t *>(value);
        Material * mat = static_cast<Material *>(clientData);
        mat->m_sTextureEnumUIntVal = textureIndex;
        if (textureIndex)
        {
            mat->m_specularTexture.first = TextureType::SpecularTexture_0;
            mat->m_specularTexture.second = textures[textureIndex - 1];//-1 since 0 is reserved for 'None', but 'None' is not in the vector
        }
        else
        {
            mat->m_specularTexture.second.reset();
        }
    }

    void Material::EditorWrapper::GetSpecularTextureCB(void* value, void* clientData)
    {
        size_t textureIndex = static_cast<Material *>(clientData)->m_sTextureEnumUIntVal;
        *static_cast<size_t *>(value) = textureIndex;
    }

    void Material::EditorWrapper::SetNormalmapTextureCB(const void* value, void* clientData)
    {
        size_t textureIndex = *static_cast<const size_t *>(value);
        Material * mat = static_cast<Material *>(clientData);
        mat->m_nTextureEnumUIntVal = textureIndex;
        if (textureIndex)
        {
            mat->m_normalMapTexture.first = TextureType::NormalMapTexture_0;
            mat->m_normalMapTexture.second = textures[textureIndex - 1];//-1 since 0 is reserved for 'None', but 'None' is not in the vector
        }
        else
        {
            mat->m_normalMapTexture.second.reset();
        }
    }

    void Material::EditorWrapper::GetNormalmapTextureCB(void* value, void* clientData)
    {
        size_t textureIndex = static_cast<Material *>(clientData)->m_nTextureEnumUIntVal;
        *static_cast<size_t *>(value) = textureIndex;
    }


}
