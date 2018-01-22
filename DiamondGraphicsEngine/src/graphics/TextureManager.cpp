#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"
#define TEXTURE_TYPE_ENUM_CHECK() static_assert(static_cast<int>(Graphics::TextureType::Count) < Graphics::NumberAvailableTextureUnits, "Too many texture types!");
namespace
{
    // Kept as a file-scope global so that it can be returned as a const ref.
    std::shared_ptr<Graphics::Texture> const NullTexture = nullptr;
    TEXTURE_TYPE_ENUM_CHECK()
}

namespace Graphics
{
    std::shared_ptr<Texture> const& TextureManager::RegisterTexture(std::string const& textureFilePath)
    {
        std::string textureName;
#if 0//now we just simply use the path as name

        //unwarp texture name to get file name;
        //backward looping to find the texture name
        for (auto iter = textureFilePath.end() - 1; iter >= textureFilePath.begin(); --iter)
        {
            if (*iter == '\\' || *iter == '/')
            {
                textureName = std::string(iter, textureFilePath.end());
                break;
            }
        }
#else//now we just simply use the path as name
        textureName = textureFilePath;
#endif // 0



        // load the texture given the specified image file; save the texture given
        // the specified textureName or, if one is already associated with that textureName,
        // replace it with the newly loaded texture
        auto find = m_textures.find(textureName);
        auto texture = Texture::LoadTGA(textureFilePath);
        if (find != m_textures.end())
        {
            find->second = texture; // replace texture
            texture->SetTextureName(textureName);
            return find->second; // and return it
        }
        else // new texture: insert(std::pair<textureName, ...>(textureName, texture))
        {
            texture->SetTextureName(textureName);
            return m_textures.emplace(textureName, texture).first->second; // and return it
        }
    }

    std::shared_ptr<Texture> const& TextureManager::RegisterTexture(
        std::string  const&textureName, std::shared_ptr<Texture> const& texture)
    {
        static std::shared_ptr<Texture> NullTexture = nullptr;
        auto emplaced = m_textures.emplace(textureName, texture);
        texture->SetTextureName(textureName);
        return emplaced.second ? emplaced.first->second : NullTexture;
    }

    std::shared_ptr<Texture> const& TextureManager::GetTexture(
        std::string textureName) const
    {
        // find a texture given the specified textureName; if it doesn't exist, return
        // null instead
        auto find = m_textures.find(textureName);
        return (find != m_textures.end()) ? find->second : NullTexture;
    }

    void TextureManager::BindTexture(std::shared_ptr<Texture> texture, std::shared_ptr<ShaderProgram> const& program,
        std::string const& samplerUniformName, TextureType slot)
    {
        int slotInt = static_cast<int>(slot);
        glActiveTexture(GL_TEXTURE0 + slotInt); // bind texture to slot
        glBindTexture(GL_TEXTURE_2D, texture->m_textureHandle);
        program->SetUniform(samplerUniformName, slotInt);
    }
    void TextureManager::BindTexture(std::string const& name, std::shared_ptr<ShaderProgram> const& program,
        std::string const& samplerUniformName, TextureType slot)
    {
        auto ifFound = m_textures.find(name);
        Assert(ifFound != m_textures.end(),"Cannot bind a texture that is not in texture manager.");
        std::shared_ptr<Texture> texture = ifFound->second;
        BindTexture(texture, program, samplerUniformName, slot);
    }

    void TextureManager::UnbindAll()
    {
        for (int i = 0; i < static_cast<int>(TextureType::Count); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // unbind texture from slot
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    void TextureManager::ClearTextures()
    {
        // free up resources consumed by registered textures
        m_textures.clear();
    }

    std::vector<std::shared_ptr<Texture>> TextureManager::GetAllTextures() const
    {
        std::vector<std::shared_ptr<Texture>> textures;
        for (auto& i : m_textures)
        {
            textures.push_back(i.second);
        }
        return textures;
    }
}
