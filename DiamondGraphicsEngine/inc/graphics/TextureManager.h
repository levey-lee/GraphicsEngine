#ifndef H_TEXTURE_MANAGER
#define H_TEXTURE_MANAGER

namespace Graphics
{
    
    //used for binding texture to specific slots of OpenGL
    enum class TextureType : int
    {
        DiffuseTexture_0,
        SpecularTexture_0,
        NormalMapTexture_0,
        
        Count
    };

    class ShaderProgram;
    class Texture;

    /*******************************************************
     * @brief 
     * This is the number of slots available in this particular instance of
     * OpenGL for multitexturing. Ie, it's the total number of textures that can
     * simultaneously be bound and access at a time in a shader. This number is
     * usually 16 or 32.
     *******************************************************/
    static u8 const NumberAvailableTextureUnits = 16;

    /*******************************************************
     * @brief 
     * Similar in purpose to the ShaderManager. Handling multiple textures at
     * once in OpenGL is its own beast. This manager helps provide an easy way to
     * deal with multiple textures being bound at once.
     *******************************************************/
    class TextureManager
    {
    public:

        TextureManager()= default;
        ~TextureManager()=default; // delete all registered textures (and unbind them)

        /*******************************************************
         * @brief Loads a texture and stores it.
         * @param textureFilePath File path of a texture, from Asset folder.
         * @return Pointer to texture, or nullptr if placement failed.
         *******************************************************/
        std::shared_ptr<Texture> const& RegisterTexture(std::string const& textureFilePath);
        /*******************************************************
         * @brief Load textures in parallel.(Multi-threads)
         * @param textureFilePaths A list of all texture file names that we are gonna load parallelly
         * @return A map of texture pointers loaded.
         *******************************************************/
        std::map<std::string /*textureName*/, std::shared_ptr<Texture> >& RegisterTextureMultiThread(std::vector<std::string> const& textureFilePaths);
        /*******************************************************
         * @brief Load textures while in other threads but main thread is not blocked.
         * @param textureFilePaths A list of all texture file names that we are gonna load parallelly
         * @return A list of texture pointers loaded.
         *******************************************************/
        std::map<std::string /*textureName*/, std::shared_ptr<Texture> >& LoadTextureMultiThreadRealTime(std::vector<std::string> const& textureFilePaths);

        /*******************************************************
         * @brief Puts an existing texture in the manager's container.
         * @param textureName Registered name of a texture.
         * @param texture Existing texture pointer
         * @return Pointer to texture, or nullptr if placement failed.
         *******************************************************/
        std::shared_ptr<Texture> const& RegisterTexture(std::string const& textureName,
                                                        std::shared_ptr<Texture> const& texture);

        /*******************************************************
         * @brief 
         * Retreives a texture based on its TextureType. If the type is unknown,
         * this method returns nullptr.
         * @param textureName Registered name of a texture.
         * @return Pointer to texture, or nullptr if texture doesn't exist.
         *******************************************************/
        std::shared_ptr<Texture> const& GetTexture(std::string textureName) const;

        /*******************************************************
         * @brief 
         * Binds a texture stored within this manager. It
         * will also attach the texture to a shader program given the uniform name
         * of the sampler the texture should be bound to. This should be the only
         * method needed to prepare shader programs to use a particular texture.
         * You don't need to worry about unbinding since the graphics engine
         * does unbind all textures after rendering one object.
         * @param texture The texture used to bind.
         * @param program Shader progrom to bind, must be bound.
         * @param samplerUniformName Uniform name of sampler2D in shader
         * @param slot OpenGL slot for binding, from GL_TEXTURE0 to GL_TEXTURE31.
         * The framework uses texture type for binding so that each type of texture
         * will have a fixed slot so it will be easier if optimization is needed.
         *******************************************************/
        static void BindTexture(std::shared_ptr<Texture> texture, std::shared_ptr<ShaderProgram> const& program,
                         std::string const& samplerUniformName, TextureType slot);

        /*******************************************************
         * @brief Check if there's any thread finished loading texture.
         * If it finishes, the function will pop the top one and return it.
         * @return On success, a pointer to a loaded texture.
         *         On failure, nullptr.
         *******************************************************/
        size_t ProcessThreadLoadedTexture();


        /*******************************************************
        * @brief
        * Binds a texture stored within this manager. It
        * will also attach the texture to a shader program given the uniform name
        * of the sampler the texture should be bound to. This should be the only
        * method needed to prepare shader programs to use a particular texture.
        * You don't need to worry about unbinding since the graphics engine
        * does unbind all textures after rendering one object.
        * @param name The name used to register the texture so it can be retrived.
        * @param program Shader progrom to bind, must be bound.
        * @param samplerUniformName Uniform name of sampler2D in shader
        * @param slot OpenGL slot for binding, from GL_TEXTURE0 to GL_TEXTURE31.
        * The framework uses texture type for binding so that each type of texture
        * will have a fixed slot so it will be easier if optimization is needed.
        *******************************************************/
        void BindTexture(std::string const& name, std::shared_ptr<ShaderProgram> const& program,
                         std::string const& samplerUniformName, TextureType slot);

        /*******************************************************
         * @brief Unbind all texture, this is called after rendering an object.
         *******************************************************/
        static void UnbindAll();

        /*******************************************************
         * @brief 
         * Destroys all textures stored within this manager. This is handy to
         * cleanup any texture resources being used by the application.
         *******************************************************/
        void ClearTextures();

        std::vector<std::shared_ptr<Texture> > GetAllTextures() const;
        std::map<std::string /*textureName*/, std::shared_ptr<Texture>>& GetTextureMap() { return m_textures; }
    private:
        void loadTextureThreadFunc(std::string const& path);

        // Disallow copying of this object.
        TextureManager(TextureManager const&) = delete;
        TextureManager& operator=(TextureManager const&) = delete;

        //used a map so the textures will be sorted. Texture in the same folder will be placed closely.
        std::map<std::string /*textureName*/, std::shared_ptr<Texture>> m_textures;
        
        //when a thread finishes loading a texture, it will put the texture in the queue
        //so that the main thread can pop and build it.
        std::list<std::shared_ptr<Texture> > m_loadingTextureQueue;

        std::shared_mutex m_mapMutex;

        size_t m_remainingTextureToLoad = 0;
    };
}

#endif
