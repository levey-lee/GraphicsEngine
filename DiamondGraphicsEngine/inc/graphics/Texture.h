#ifndef H_TEXTURE
#define H_TEXTURE

#include "framework/Utilities.h"
#include "graphics/Color.h"

#pragma pack(push)
#pragma pack(1)

namespace Graphics
{
    class ShaderProgram;

	/*******************************************************************
     * @brief 
     * By far, one of the best explanations I've come across that explains how
     * the concept of multitexturing works in OpenGL (the accepted answer):
     *   http://stackoverflow.com/questions/8866904
     * This class represents an OpenGL texture, loaded from a PNG or TGA file.
     * Textures may be bound, attached to a uniform in a shader (sampler type),
     * or unbound. They are similar in concept to other types of OpenGL objects,
     * though interacting with them on the shader is different, given they have a
     * much different purpose to serve.
     ******************************************************************/
    class Texture
    {
		friend class TextureManager;
	public:
        enum class Format
        {
            RGB, RGBA,
            COUNT
        };

        // Texture holds 24-bit colors, so we only have 3 8-bit channels for RGB.
        struct IntColor
        {
            // NOTE: The alpha channel is NOT valid to read from when IntColor is
            // provided from an RGB texture. It will overflow and either go out of
            // bounds or read the red channel of the neighboring pixel. IntColors
            // written to an RGB texture will have their alpha values ignored.
            u8 r, g, b, a; // red, green, blue, alpha; order matters

            // implicit conversion from Color to IntColor
            IntColor(Color const &color)
                : r(static_cast<u8>(color.r * 255.f)),
                g(static_cast<u8>(color.g * 255.f)),
                b(static_cast<u8>(color.b * 255.f)),
                a(static_cast<u8>(color.a * 255.f))
            {
            }
        };

        // Create a new empty texture (default pixels are filled to black).
        Texture(u32 width, u32 height, Format format = Format::RGB);
        // this Texture now owns pix
        Texture(u8 *pix, u32 width, u32 height, Format format);
        ~Texture();

        // builds the texture and uploads it to the graphics card
        void Build();
        bool IsBuilt() const { return m_isBuilt; }
        void DownloadContents(); // downloads pixel data from GPU (bind first)
        bool IsBound() const;
        void Unbind();
        void Destroy();
        u32 GetWidth() const;
        u32 GetHeight() const;
        
        u32 GetTextureHandle() const;

        Format GetFormat() const;
        bool HasAlpha() const;

        // Get the number of bytes per pixel. 3 means RGB, 4 means RGBA.
        u8 GetBPP() const;

        // Do NOT access a if HasAlpha() returns false.
        IntColor const* GetPixel(u32 x, u32 y) const;

        // Do NOT access or change a if HasAlpha() returns false.
        IntColor* GetPixel(u32 x, u32 y);

        // Color's alpha value ignored if this is a RGB texture.
        void SetPixel(u32 x, u32 y, IntColor const& color);

        static std::shared_ptr<Texture> LoadTGA(std::string const &path);
        static std::shared_ptr<Texture> LoadPNG(std::string const &path);
        static void SavePNG(std::shared_ptr<Texture> const &texture,
            std::string const &path);
        static void SavePNG(Texture const *texture, std::string const &path);

        std::string const& GetTextureName() const { return m_textureName; }
        void SetTextureName(std::string const& name) { m_textureName = name; }

    private:
        u8 *m_pixels;
        u32 m_width;
        u32 m_height;
        u32 m_textureHandle;
        int m_boundSlot;
        u8 m_bpp;
        Format m_format;
        std::string m_textureName;
        bool m_isBuilt = false;
    };
}

#pragma pack(pop)

#endif
