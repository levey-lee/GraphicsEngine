#ifndef H_SHADER_MANAGER
#define H_SHADER_MANAGER

namespace Graphics
{
    enum class ShaderUsage;
    class Shader;
    class ShaderProgram;

    enum class ShaderStage : unsigned char
    {
        ForwardRendering = 0,

        DiffuseMaterial = 0,
        SSAO = 1,
        SSAOBlurH = 2,
        SSAOBlurV = 3,
        DeferredLighting = 4,
        ShadowBlurH = 5,
        ShadowBlurV = 6,
        RenderFullScreenQuad = 7,

        Count
    };

    // This enum stores all the known types of shaders used by the application.
    // Add more to this enum as you create more shader programs.
    enum class ShaderType
    {
        /*******************************************************
         * @brief
         * This framework uses ubershader with different materials. Ubershader is just one giant shader
         * that has many different branches. Some people may suggest that using many small shaders is a
         * better choice, but you need to do more work in this framework.(i.e. add shader specific code in
         * every shaded component, change vertex layout etc). The framework DOES support multiple shaders.
         * One of the best comparasion I can find on the internet is the following.
         * https://gamedev.stackexchange.com/questions/34883/glsl-all-in-one-or-many-shader-programs
         *******************************************************/
        UberForward = 0,
        UberDeferred = 0,
        //TODO(STUDENT): Add more shaders if you don't want to use uber shader.
        Count,

        //reserved for not shading an object
        Null = -1,
    };

    /*******************************************************
     * @brief 
     * This class is responsible for storing all shader programs loaded within
     * the framework. For more information on what a shader program is or how
     * they work, please refer to ShaderProgram.h. This class associates a shader
     * program with a type (as defined in the ShaderType enum above), simplifying
     * shader usage throughout the application. 
     *******************************************************/
    class ShaderManager
    {
    public:
        ShaderManager();
        ~ShaderManager();
        /*******************************************************
         * @brief 
         * Registers a new shader given its source vertex and fragment shader files,
         * as well as the type it should be bound to. If a shader has already been
         * loaded to a specific type.
         * @param shaderType Forward shader type.
         * @param vertexSourceFile Relative file path to Asset/Shaders/
         * @param fragmentSourceFile Relative file path to Asset/Shaders/
         * @return A pointer to the new shader.
         * @note This is for forward rendering.
         *******************************************************/
        std::shared_ptr<Shader> LoadShader(ShaderType shaderType,  std::string const& vertexSourceFile,
                                                std::string const& fragmentSourceFile);


        /*******************************************************
         * @brief Load a series of shaders in one shader, which is deferred shader.
         * @param shaderType Deferred shader type.
         * @param shaderFiles A vector of all files that will be used
         * as shading stages in a deferred pipeline. Stage order is by 
         * vector index.
         * @return A pointer to the new shader.
         * @note This is for deferred rendering.
         *******************************************************/
        std::shared_ptr<Shader> LoadShader(ShaderType shaderType,
                                                std::vector<std::tuple<
                                                std::string /*vertexSourceFile*/,
                                                std::string /*fragmentSourceFile*/,
                                                ShaderUsage>>const& shaderFiles);

        // Retreives a shader program based on its ShaderType. If the type is
        // unknown, this method returns NULL.
        /*******************************************************
         * @brief Retreives a shader program based on its ShaderType. If the type is
         *        unknown, this method returns NULL.
         * @param type Type of the shader
         * @return A pointer to the shader.
         *******************************************************/
        std::shared_ptr<Shader> const& GetShader(ShaderType type) const;

        /*******************************************************
         * @brief Destroys all shader programs stored within this manager. This is handy
         *        to cleanup any shader resources being used by the application.
         *******************************************************/
        void ClearAllShaders();

        /*******************************************************
         * @brief Unbind all shaders.
         *  This is not static for encapsulation purpose.
         *******************************************************/
        void UnbindAllShader() { glUseProgram(NULL); }

    private:
        // Disallow copying of this object.
        ShaderManager(ShaderManager const&) = delete;
        ShaderManager& operator=(ShaderManager const&) = delete;

        std::unordered_map<ShaderType, std::shared_ptr<Shader> > m_shaders;

    };
}

#endif
