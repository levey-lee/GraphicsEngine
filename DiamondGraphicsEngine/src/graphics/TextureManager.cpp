#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"
#define TEXTURE_TYPE_ENUM_CHECK static_assert(static_cast<int>(Graphics::TextureType::Count) < Graphics::NumberAvailableTextureUnits, "Too many texture types!");
#define LOADING_TEXTURE_FILE "Loading.jpg"
#define ERROR_TEXTURE_FILE "error.jpg"
namespace
{
    // Kept as a file-scope global so that it can be returned as a const ref.
    std::shared_ptr<Graphics::Texture> const NullTexture = nullptr;
    TEXTURE_TYPE_ENUM_CHECK
}

namespace Graphics
{
    std::shared_ptr<Texture> const& TextureManager::RegisterTexture(std::string const& textureFilePath)
    {
        std::string const& textureName = textureFilePath;

        // load the texture given the specified image file; save the texture given
        // the specified textureName or, if one is already associated with that textureName,
        // replace it with the newly loaded texture
        auto find = m_textures.find(textureName);
        auto texture = Texture::LoadFromFile(textureFilePath);
        if (find != m_textures.end())
        {
            find->second = texture; // replace texture
            texture->SetTextureName(textureName);
            find->second->Build();
            return find->second; // and return it
        }
        else // new texture: insert(std::pair<textureName, ...>(textureName, texture))
        {
            texture->SetTextureName(textureName);
            find->second->Build();
            return m_textures.emplace(textureName, texture).first->second; // and return it
        }
    }

    std::map<std::string /*textureName*/, std::shared_ptr<Texture> > & TextureManager::RegisterTextureMultiThread(
        std::vector<std::string> const& textureFilePaths)
    {
        std::vector<std::thread > threads;
        size_t threadNum = textureFilePaths.size();
        threads.resize(threadNum);
        m_remainingTextureToLoad = threadNum;

        //load textures in parallel
        for (size_t i = 0; i < threadNum; ++i)
        {
            threads[i] = std::thread(&TextureManager::loadTextureThreadFunc, this, textureFilePaths[i]);
        }
        //join all threads and build all textures
        for (size_t i = 0; i < threadNum; ++i)
        {
            threads[i].join();
        }
        for (auto& i : m_loadingTextureQueue)
        {
            i->Build();
            m_textures[i->GetTextureName()] = i;
        }
        //clear the queue since we manually built the textures above;
        m_loadingTextureQueue.clear();
        //std::cout << glutGet(GLUT_ELAPSED_TIME)*0.001f << std::endl;
        return m_textures;
    }

    std::map<std::string /*textureName*/, std::shared_ptr<Texture> >& TextureManager::LoadTextureMultiThreadRealTime(std::vector<std::string> const& textureFilePaths)
    {
        size_t threadNum = textureFilePaths.size();
        m_remainingTextureToLoad = threadNum;

        //create all textures
        for (size_t i = 0; i < threadNum; ++i)
        {
            std::shared_ptr<Texture> ptex = Texture::LoadFromFile("error.jpg");
            ptex->Build();
            ptex->SetTextureName(textureFilePaths[i]);
            m_textures[textureFilePaths[i]] = ptex;
        }

        std::vector<std::thread > threads;
        threads.resize(threadNum);
        //load textures in parallel
        for (size_t i = 0; i < threadNum; ++i)
        {
            //std::thread t(&TextureManager::loadTextureThreadFunc, this, textureFilePaths[i]);
            threads[i] = std::thread(&TextureManager::loadTextureThreadFunc, this, textureFilePaths[i]);
        }
        for (size_t i = 0; i < threadNum; ++i)
        {
            threads[i].detach();
        }
        return m_textures;
    }

    std::shared_ptr<Texture> const& TextureManager::RegisterTexture(
        std::string  const&textureName, std::shared_ptr<Texture> const& texture)
    {
        static std::shared_ptr<Texture> NullTexture = nullptr;
        auto emplaced = m_textures.emplace(textureName, texture);
        texture->SetTextureName(textureName);
        texture->Build();
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

    //thread loading
    void TextureManager::loadTextureThreadFunc(std::string const& path)
    {
        std::shared_ptr<Texture> loadedTexture = Texture::LoadFromFile(path);
        loadedTexture->SetTextureName(path);
        std::lock_guard<std::shared_mutex> lockguard(m_mapMutex);
        m_loadingTextureQueue.push_back(loadedTexture);
#if VERBOSE
        std::cout << "A thread finished loading texture with name: \"" + path + "\".  -- " + std::to_string(--m_remainingTextureToLoad) + " Remain.\n";
#endif // VERBOSE
    }

    //called only by main thread
    size_t TextureManager::ProcessThreadLoadedTexture()
    {
        std::lock_guard<std::shared_mutex> guard(m_mapMutex);
        while (!m_loadingTextureQueue.empty())
        {
            std::shared_ptr<Texture> texture = m_loadingTextureQueue.front();
            m_loadingTextureQueue.pop_front();
            m_textures.at(texture->GetTextureName())->ReplaceAndBuild(texture);
        }
        return m_remainingTextureToLoad;
    }
}
