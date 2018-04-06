#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/Color.h"
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"
#include "graphics/ShaderProgram.h"

namespace Graphics
{
    Framebuffer::Framebuffer(u32 width, u32 height)
        : m_width(width), m_height(height)
    {
        static_assert(static_cast<u8>(GBufferAttachmentType::Count) <= MXA_FBO_ATTACHMENT_NUM,"Too many frame buffer attachments.");
    }

    void Framebuffer::Resize(u32 width, u32 height, bool depthOnly)
    {
        m_width = width;
        m_height = height;
        Destroy();
        Build(m_usage);
    }

    void Framebuffer::Build(FBO_USAGE usage)
    {
        m_usage = usage;
        // build framebuffer
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        if (usage  == FBO_USAGE_REGULAR)
        {
            // build textures
            int counter = 0;
            for (auto & i : m_colorTexture)
            {
                size_t textureDataSize = sizeof(u8) * m_width * m_height * 4;
                u8* data = new u8[textureDataSize];
                std::memset(data, 0, textureDataSize);
                i = std::make_shared<Texture>(data, m_width, m_height, Texture::Format::RGBA);
                i->Build();

                // create a new texture
                glGenTextures(1, &(*i).m_textureHandle);
                // bind the generated texture and upload its image contents to OpenGL
                glBindTexture(GL_TEXTURE_2D, i->m_textureHandle);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
                if (counter == 1)//this is for position texture
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
                }
                else
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                }
                i->m_isBuilt = true;
                ++counter;
            }
            
            {//generate depth buffer
                glGenTextures(1, &m_depthTextureHandle);
                glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTextureHandle, 0);
            }

            for (u8 i = 0; i < static_cast<u8>(GBufferAttachmentType::Count); i++)
            {
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_colorTexture[i]->GetTextureHandle(), 0);
            }

            GLenum DrawBuffers[4] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
            };
            glDrawBuffers(4, DrawBuffers); // size of DrawBuffers

        }
        else if (usage == FBO_USAGE_FLOAT_BUFFER)//float buffer
        {
            glGenTextures(1, &m_depthTextureHandle);
            glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            }
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTextureHandle, 0);

            glGenTextures(1, &m_floatBuffer);
            glBindTexture(GL_TEXTURE_2D, m_floatBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_width, m_height, 0, GL_RED, GL_FLOAT, nullptr);
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            }
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<u8>(GBufferAttachmentType::DiffuseColor_TexU), m_floatBuffer, 0);

            GLenum DrawBuffers[1] = {
                GL_COLOR_ATTACHMENT0 + static_cast<u8>(GBufferAttachmentType::DiffuseColor_TexU),
            };
            glDrawBuffers(1, DrawBuffers); // size of DrawBuffers
        }
        else if (usage == FBO_USAGE_DEPTH_BUFFER)
        {
            glGenTextures(1, &m_depthTextureHandle);
            glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTextureHandle, 0);
        }


        // verify it worked
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        Assert(status == GL_FRAMEBUFFER_COMPLETE, "Failed to create framebuffer.");
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.f, 0.f, 0.f, 1.f); // default clear color
        glClearDepth(1.f); // default clear depth
        Unbind();

        CheckGL();
    }


    void Framebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_width, m_height);
        WarnIf(m_fbo == NULL, "Warning: Binding unbuilt framebuffer.");
    }

    void Framebuffer::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    std::shared_ptr<Texture> const& Framebuffer::GetFboColorAttachment(u8 attachmentIndex) const
    {
        Assert(attachmentIndex < static_cast<u8>(GBufferAttachmentType::Count),"Invalid attachment index passed: %d", attachmentIndex);
        return m_colorTexture[attachmentIndex];
    }

    std::shared_ptr<Texture> const& Framebuffer::GetFboColorAttachment(GBufferAttachmentType type) const
    {
        Assert(type != GBufferAttachmentType::Count, "Invalid type passed to passed.");
        return m_colorTexture[static_cast<u8>(type)];
    }

    void Framebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind screen framebuffer
    }

    void Framebuffer::Destroy()
    {
        Unbind();
        for (auto & i : m_colorTexture)
        {
            if (i != nullptr)
            {
                i->Destroy();
                i = nullptr; // deletes it 
            }
        }
        glDeleteTextures(1, &m_depthTextureHandle);
        glDeleteTextures(1, &m_floatBuffer);
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = NULL;
        m_depthTextureHandle = NULL;

    }


    Framebuffer* Framebuffer::BindGBufferTextures(const std::shared_ptr<ShaderProgram>& shaderProgram) 
    {
        Assert(m_colorTexture[0]->GetTextureHandle() != 0, "Invalid FBO Binding");
        Assert(m_colorTexture[1]->GetTextureHandle() != 0, "Invalid FBO Binding");
        Assert(m_colorTexture[2]->GetTextureHandle() != 0, "Invalid FBO Binding");
        Assert(m_colorTexture[3]->GetTextureHandle() != 0, "Invalid FBO Binding");

        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::DiffuseColor_TexU));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[0]->GetTextureHandle());
        shaderProgram->SetUniform("DiffuseColor_Empty_Texture", static_cast<u8>(GBufferAttachmentType::DiffuseColor_TexU));

        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::WorldPosition_TexV));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[1]->GetTextureHandle());
        shaderProgram->SetUniform("WorldPosition_SpecPow_Texture", static_cast<u8>(GBufferAttachmentType::WorldPosition_TexV));

        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::WorldNormal_ReceiveLight));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[2]->GetTextureHandle());
        shaderProgram->SetUniform("WorldNormal_ReceiveLight_Texture", static_cast<u8>(GBufferAttachmentType::WorldNormal_ReceiveLight));

        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::SpecColor_SpecPow));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[3]->GetTextureHandle());
        shaderProgram->SetUniform("SpecColor_Empty_Texture", static_cast<u8>(GBufferAttachmentType::SpecColor_SpecPow));

        return this;
    }

    Framebuffer* Framebuffer::BindGBufferPositionNormal(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        Assert(m_colorTexture[1]->GetTextureHandle() != 0, "Invalid FBO Binding");
        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::WorldPosition_TexV));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[1]->GetTextureHandle());
        shaderProgram->SetUniform("WorldPosition_TexV_Texture", static_cast<u8>(GBufferAttachmentType::WorldPosition_TexV));

        Assert(m_colorTexture[2]->GetTextureHandle() != 0, "Invalid FBO Binding");
        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::WorldNormal_ReceiveLight));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[2]->GetTextureHandle());
        shaderProgram->SetUniform("WorldNormal_ReceiveLight_Texture", static_cast<u8>(GBufferAttachmentType::WorldNormal_ReceiveLight));
        return this;
    }

    Framebuffer* Framebuffer::BindGBufferNormal(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        Assert(m_colorTexture[2]->GetTextureHandle() != 0, "Invalid FBO Binding");
        glActiveTexture(GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::WorldNormal_ReceiveLight));
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[2]->GetTextureHandle());
        shaderProgram->SetUniform("WorldNormal_ReceiveLight_Texture", static_cast<u8>(GBufferAttachmentType::WorldNormal_ReceiveLight));
        return this;
    }

    Framebuffer* Framebuffer::BindDepthTexture(const std::shared_ptr<ShaderProgram>& shaderProgram) 
    {
        Assert(m_depthTextureHandle != 0, "Invalid FBO Binding");
		auto num = GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::DepthTexture);
        glActiveTexture(num);
        glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
        shaderProgram->SetUniform("Depth_Texture", static_cast<u8>(GBufferAttachmentType::DepthTexture));

        return this;
    }
    Framebuffer* Framebuffer::BindShadowMapTexture(const std::shared_ptr<ShaderProgram>& shaderProgram) 
    {
        Assert(m_floatBuffer != 0, "Invalid FBO Binding");
        auto num = GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::ShadowMap);
        glActiveTexture(num);
        glBindTexture(GL_TEXTURE_2D, m_floatBuffer);
        shaderProgram->SetUniform("ShadowMaps_Texture", static_cast<u8>(GBufferAttachmentType::ShadowMap));

        return this;
    }

    Framebuffer* Framebuffer::BindSSAOTexture(const std::shared_ptr<ShaderProgram>& shaderProgram)
    {
        Assert(m_depthTextureHandle != 0, "Invalid FBO Binding");
        auto num = GL_TEXTURE0 + static_cast<u8>(GBufferAttachmentType::SSAO);
        glActiveTexture(num);
        glBindTexture(GL_TEXTURE_2D, m_depthTextureHandle);
        shaderProgram->SetUniform("SSAO_Texture", static_cast<u8>(GBufferAttachmentType::SSAO));

        return this;
    }
}
