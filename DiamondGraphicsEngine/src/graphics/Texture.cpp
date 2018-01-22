#include "Precompiled.h"
#include "framework/Debug.h"
#include "graphics/ShaderProgram.h"
#include "graphics/Texture.h"

#include <STB/stb_image.h>
#include <STB/stb_image_write.h>

static u8 const UnbuiltTexture = 0;
static u8 const UnboundTexture = -1;

// Converts the relative path to a useful relative path (based on the local
// directory of the executable). This assumes ASSET_PATH is correct.
static std::string GetFilePath(std::string const &relativePath)
{
  std::stringstream strstr;
  strstr << ASSET_PATH << "textures/" << relativePath;
  return strstr.str();
}

namespace Graphics
{
    Texture::Texture(u32 width, u32 height, Format format/* = Format::RGB*/)
        : m_pixels(new u8[width * height * sizeof(IntColor)]), m_width(width),
        m_height(height), m_textureHandle(UnbuiltTexture), m_boundSlot(UnboundTexture),
        m_bpp(format == Format::RGB ? 3 : 4), m_format(format)
    {
        // fill pixels to black
        std::memset(m_pixels, 0, width * height * m_bpp * sizeof(u8));
    }

    Texture::Texture(u8 *pixels, u32 width, u32 height, Format format)
        : m_pixels(pixels), m_width(width), m_height(height),
        m_textureHandle(UnbuiltTexture), m_boundSlot(UnboundTexture),
        m_bpp(format == Format::RGB ? 3 : 4), m_format(format)
    {
    }

    Texture::~Texture()
    {
        Destroy();
        delete[] m_pixels;
    }

    void Texture::Build()
    {
        Assert(m_textureHandle == UnbuiltTexture,
            "Cannot build already built texture.");

        GLenum format = (m_format == Format::RGB) ? GL_RGB : GL_RGBA;

        // create a new texture
        glGenTextures(1, &m_textureHandle);

        // bind the generated texture and upload its image contents to OpenGL
        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, m_pixels);

        // unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);
#ifdef _DEBUG
		CheckGL();
#endif // _DEBUG
        m_isBuilt = true;
    }

    void Texture::DownloadContents()
    {
        GLenum format = (m_format == Format::RGB) ? GL_RGB : GL_RGBA;
        glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, m_pixels);
    }

    bool Texture::IsBound() const
    {
        return m_boundSlot != UnboundTexture;
    }


    void Texture::Unbind()
    {
        WarnIf(m_boundSlot == UnboundTexture,
            "Warning: Cannot unbind unbound texture.");
        glActiveTexture(GL_TEXTURE0 + m_boundSlot); // unbind texture from slot
        glBindTexture(GL_TEXTURE_2D, 0);
        m_boundSlot = UnboundTexture; // unbound
    }

    void Texture::Destroy()
    {
        if (m_textureHandle == UnbuiltTexture)
            return; // nothing to destroy
        if (m_boundSlot != UnboundTexture)
            Unbind();
        glDeleteTextures(1, &m_textureHandle); // wipe out the texture
        m_textureHandle = UnbuiltTexture;
    }

    u32 Texture::GetWidth() const
    {
        return m_width;
    }

    u32 Texture::GetHeight() const
    {
        return m_height;
    }

    u32 Texture::GetTextureHandle() const
    {
        return m_textureHandle;
    }

    Texture::Format Texture::GetFormat() const
    {
        return m_format;
    }

    bool Texture::HasAlpha() const
    {
        return m_format == Format::RGBA;
    }

    u8 Texture::GetBPP() const
    {
        return m_bpp;
    }

    Texture::IntColor const* Texture::GetPixel(u32 x, u32 y) const
    {
        return &reinterpret_cast<IntColor const *>(m_pixels)[(y * m_width) + x];
    }

    void Texture::SetPixel(u32 x, u32 y, IntColor const& color)
    {
        u8* channels = m_pixels + (((y * m_width) + x) * m_bpp);
        *(channels + 0) = color.r;
        *(channels + 1) = color.g;
        *(channels + 2) = color.b;
        if (m_format == Format::RGBA)
            *(channels + 3) = color.a;
    }

    Texture::IntColor* Texture::GetPixel(u32 x, u32 y)
    {
        size_t offset = ((y * m_width) + x) * m_bpp;
        return reinterpret_cast<IntColor *>(m_pixels + offset);
    }

    std::shared_ptr<Texture> Texture::LoadTGA(std::string const &path)
    {
        // STB image handles the type based on the format itself
        return LoadPNG(path);
    }

    std::shared_ptr<Texture> Texture::LoadPNG(std::string const &relative)
    {
        // convert relative path (to textures) to a fully qualified relative path
        // (relative to the executable itself)
        std::string path = GetFilePath(relative);

        // attempt to load a PNG/TGA file using STB Image
        int width = 0, height = 0, bpp = 0;
        void *data = stbi_load(path.c_str(), &width, &height, &bpp, STBI_rgb);

        Assert(data, "Error: Unable to read file: textures/%s, reason: %s",
            relative.c_str(), stbi_failure_reason());
        if (!data)
            return nullptr; // failed to load

        Assert(bpp == 3, "Error: Can only handle RGB images in the CS300 framework."
            " No alpha channels supported. Read file with bpp=%d", bpp);

        Texture *texture = nullptr;
        if (bpp == 3) // successfully read an image with 3 channels of data
        {
            // copy data from loaded STB image buffer to local pixel buffer
            u8 *pixelData = new u8[width * height * bpp];
            std::memcpy(pixelData, data, width * height * bpp);
            texture = new Texture(pixelData, u32(width), u32(height), Format::RGB);
        }

        stbi_image_free(data);
        return std::shared_ptr<Texture>(texture);
    }

    void Texture::SavePNG(std::shared_ptr<Texture> const &texture,
        std::string const &path)
    {
        SavePNG(texture.get(), path);
    }

    void Texture::SavePNG(Texture const *texture, std::string const &path)
    {
        u32 width = texture->m_width;
        u32 compCount = (texture->m_format == Format::RGB) ? 3 : 4;
        stbi_write_png(path.c_str(), width, texture->m_height,
            compCount, texture->m_pixels, width * compCount * sizeof(u8));
    }
}
