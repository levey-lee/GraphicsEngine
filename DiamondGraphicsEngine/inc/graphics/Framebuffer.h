#ifndef H_FRAMEBUFFER
#define H_FRAMEBUFFER

#include "graphics/Color.h"
#include "framework/Utilities.h"

namespace Graphics
{
    enum class TextureType;
    class ShaderProgram;
    class Texture;

    enum class GBufferAttachmentType : u8
    {
        //XYZ = Diffuse, W = U
        DiffuseColor_TexU,

        //XYZ = Position, W = V
        WorldPosition_TexV,

        //XYZ = Normal, W = If ReceiveLight
        WorldNormal_ReceiveLight,
        
        //XYZ = Specular Color, W = Specular Color
        SpecColor_SpecPow,

        Count,
        //this is intentional, since depth texture is 
        //not a color texture attachment of RGB,
        //this value is used to bind depth texture,
        //but not to attach it in the first pass
        DepthTexture = Count,
        ShadowMap,
        FloatBuffer,
        SSAO
    };

    class Framebuffer
    {
    public:
        static const u8 MXA_FBO_ATTACHMENT_NUM = 4;
        Framebuffer(u32 width, u32 height);

        u32 GetWidth() const { return m_width; }
        u32 GetHeight() const { return m_height; }

        u32 GetHandle() const { return m_fbo; }

        void SetClearColor(Color const& color) { m_clearColor = color; }
        Color const& GetClearColor() const { return m_clearColor; }

        void Resize(u32 width, u32 height, bool depthOnly);
        void Build(bool depthOnly = false);
        void BuildSsaoBuffer();
        void Bind(); // bind for rendering
        void Clear();
        /*******************************************************
       * @brief Get a colored texture
       * @param attachmentIndex Must be from 0-AttachedTextureType::Count
       * @return A pointer to texture from FBO
       *******************************************************/
        std::shared_ptr<Texture> const& GetFboColorAttachment(u8 attachmentIndex = 0) const;
        std::shared_ptr<Texture> const& GetFboColorAttachment(GBufferAttachmentType type) const;
        void Unbind();
        void Destroy();
        Framebuffer* BindGBufferTextures(const std::shared_ptr<ShaderProgram>& shaderProgram);
        Framebuffer* BindGBufferPositionNormal(const std::shared_ptr<ShaderProgram>& shaderProgram);
        Framebuffer* BindGBufferNormal(const std::shared_ptr<ShaderProgram>& shaderProgram);
        Framebuffer* BindDepthTexture(const std::shared_ptr<ShaderProgram>& shaderProgram);
        Framebuffer* BindShadowMapTexture(const std::shared_ptr<ShaderProgram>& shaderProgram);
        Framebuffer* BindSSAOTexture(const std::shared_ptr<ShaderProgram>& shaderProgram, bool useFloatBuffer);

    private:
        void genDepthTexture(bool asShadowMap);

        u32 m_width, m_height;
        GLuint m_fbo = 0;
        GLuint m_depthTextureHandle = 0;
        GLuint m_floatBuffer = 0;
        Color m_clearColor = Color::Cyan;
        std::shared_ptr<Texture> m_colorTexture[static_cast<int>(GBufferAttachmentType::Count)];

        std::shared_ptr<Texture> m_ssaoTexture;
        bool m_usedForSSAO = false;

    };
}

#endif
