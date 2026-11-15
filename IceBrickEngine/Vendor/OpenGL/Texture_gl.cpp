#include "Texture_gl.hpp"

#include "glad/gl.h"

#include "RHI/TextureDescription.hpp"
#include "Vendor/OpenGL/TextureHelper_gl.hpp"
#include <GL/gl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace IceBrick::RHI
{
    GLTexture::GLTexture(const TextureDescription& description)
        : m_Description(description)
    {
        uint32_t mipLevels = 1;
        if(m_Description.generateMips)
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_Description.width, m_Description.height)))) + 1;

        GLenum internalFormat = TranslateInternalFormatToGL(m_Description.format);
        if(m_Description.type == TextureType::Texture2D)
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
            glTextureStorage2D(m_ID, mipLevels, internalFormat, m_Description.width, m_Description.height);
        }
        else if(m_Description.type == TextureType::Texture3D)
        {
            glCreateTextures(GL_TEXTURE_3D, 1, &m_ID);
            glTextureStorage3D(m_ID, mipLevels, internalFormat, m_Description.width, m_Description.height, m_Description.depth);
        }

        GLenum wrap = TranslateWrapToGL(m_Description.wrap);
        
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(m_ID, GL_TEXTURE_WRAP_T, wrap);
        if(m_Description.type == TextureType::Texture3D)
            glTextureParameteri(m_ID, GL_TEXTURE_WRAP_R, wrap);

        glTextureParameteri(m_ID, GL_TEXTURE_MIN_FILTER, TranslateFilterToGL(m_Description.filter, m_Description.generateMips)); 
        glTextureParameteri(m_ID, GL_TEXTURE_MAG_FILTER, TranslateFilterToGL(m_Description.filter, false));

    }

    GLTexture::~GLTexture() 
    {
        glDeleteTextures(1, &m_ID);
    }

    const TextureDescription& GLTexture::getDescription() const
    {
        return m_Description;
    }

    uint32_t GLTexture::GetWidth() const { return m_Description.width; }
    uint32_t GLTexture::GetHeight() const { return m_Description.height; }
    uint32_t GLTexture::GetDepth() const { return m_Description.depth; }

    void GLTexture::SetData(const void* data, uint32_t size)
    {
        GLenum format = TranslateFormatToGL(m_Description.format);
        GLenum type = TranslateTypeToGL(m_Description.format);

        if(m_Description.type == TextureType::Texture2D)
        {
            glTextureSubImage2D(
                    m_ID,
                    0,
                    0,
                    0,
                    m_Description.width,
                    m_Description.height,
                    format,
                    type,
                    data
            );



        }
        else if(m_Description.type == TextureType::Texture3D)
        {
            glTextureSubImage3D(
                    m_ID,
                    0,
                    0,
                    0,
                    0,
                    m_Description.width,
                    m_Description.height,
                    m_Description.depth,
                    format,
                    type,
                    data
            );
        }
        if(m_Description.generateMips)
            glGenerateTextureMipmap(m_ID);

    }

    // Because i use void* and it's a 64-bit value i can't just reinterpret cast it
    // because unsigned int is 32-bit, so i need to cast it to a uintptr_t because
    // it is guaranteed to be 64-bit integer and then i can reinterpret it to void*
    void* GLTexture::GetNativeHandle()
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(m_ID));
    }

};
