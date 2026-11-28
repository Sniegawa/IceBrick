#pragma once


#include "RHI/rhi_types.hpp"
#include <glad/gl.h>
#include <spdlog/spdlog.h>

namespace IceBrick::RHI
{
    // TEXTURE

    inline GLenum TranslateInternalFormatToGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8_UNORM:   return GL_RGBA8;
            case TextureFormat::RGBA16_FLOAT:  return GL_RGBA16F;
            case TextureFormat::RGBA32_FLOAT:  return GL_RGBA32F;
            case TextureFormat::R32_UINT:      return GL_R32UI;
            case TextureFormat::R16_UINT:      return GL_R16UI;
            case TextureFormat::Depth32_FLOAT: return GL_DEPTH_COMPONENT32F;
        }
        spdlog::error("Unknown TextureFormat in TranslateInternalFormatToGL");
        return GL_RGBA8;
    }

    inline GLenum TranslateFormatToGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8_UNORM:   return GL_RGBA;
            case TextureFormat::RGBA16_FLOAT:  return GL_RGBA;
            case TextureFormat::RGBA32_FLOAT:  return GL_RGBA;
            case TextureFormat::R32_UINT:      return GL_RED_INTEGER; 
            case TextureFormat::R16_UINT:      return GL_RED_INTEGER;
            case TextureFormat::Depth32_FLOAT: return GL_DEPTH_COMPONENT;
        }
        spdlog::error("Unknown TextureFormat in TranslateFormatToGL");
        return GL_RGBA;
    }

    inline GLenum TranslateTypeToGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8_UNORM:   return GL_UNSIGNED_BYTE;
            case TextureFormat::RGBA16_FLOAT:  return GL_HALF_FLOAT; 
            case TextureFormat::RGBA32_FLOAT:  return GL_FLOAT;
            case TextureFormat::R32_UINT:      return GL_UNSIGNED_INT;
            case TextureFormat::R16_UINT:      return GL_UNSIGNED_SHORT;
            case TextureFormat::Depth32_FLOAT: return GL_FLOAT;
        }
        spdlog::error("Unknown TextureFormat in TranslateTypeToGL");
        return GL_UNSIGNED_BYTE;
    }

    inline GLenum TranslateFilterToGL(TextureFilter filter, bool isMipmapped)
    {
        if(filter == TextureFilter::Nearest)
            return isMipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
        else if(filter == TextureFilter::Linear)
            return isMipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        else
            spdlog::error("Unknown TextureFormat in TranslateFilterToGL");
        return GL_LINEAR;
    }

    inline GLenum TranslateWrapToGL(TextureWrap wrap)
    {
        switch(wrap)
        {
            case TextureWrap::Repeat:
                return GL_REPEAT;
            case TextureWrap::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
        }
        spdlog::error("Unknown TextureWrap in TranslateWrapToGL");
        return GL_REPEAT;
    }

    constexpr size_t GetBytesPerPixel(TextureFormat format)
    {
        switch(format)
        {
            case TextureFormat::R16_UINT:           return 2;
            case TextureFormat::R32_UINT:           return 4;
            case TextureFormat::RGBA8_UNORM:        return 4;
            case TextureFormat::Depth32_FLOAT:      return 4;
            case TextureFormat::RGBA16_FLOAT:       return 8;
            case TextureFormat::RGBA32_FLOAT:       return 16;
            default:
                spdlog::error("Unknown TextureFormat in GetBytesPerPixel!");
                return 0;
        }
    }

    constexpr bool HasFlag(TextureUsage value, TextureUsage flag)
    {
        return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
    }

    // BUFFERS

    inline GLbitfield TranslateMemoryAccessToGLFlags(BufferMemoryAccess access)
    {
        GLbitfield flags = 0;

        switch(access)
        {
        case BufferMemoryAccess::GpuOnly:
            flags = GL_DYNAMIC_STORAGE_BIT;
            break;
        case BufferMemoryAccess::CpuToGpu:
            flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            break;
        case BufferMemoryAccess::GpuToCpu:
            flags = GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            break;
        }

        return flags;
    }

    // Rendering
    inline GLbitfield TranslateMemoryBarrierToGL(MemoryBarrierBits barrier)
    {
        GLbitfield flags = 0;
        const auto mask = static_cast<uint32_t>(barrier);

        if ((mask & static_cast<uint32_t>(MemoryBarrierBits::ShaderImageAccess)) != 0u)
            flags |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;

        if ((mask & static_cast<uint32_t>(MemoryBarrierBits::TextureFetch)) != 0u)
            flags |= GL_TEXTURE_FETCH_BARRIER_BIT;

        if ((mask & static_cast<uint32_t>(MemoryBarrierBits::ShaderStorage)) != 0u)
            flags |= GL_SHADER_STORAGE_BARRIER_BIT;

        return flags;
    }

    // Graphics pipeline
    inline GLenum TranslateCullModeToGL(CullMode mode)
    {
        switch (mode)
        {
            case CullMode::Front: return GL_FRONT;
            case CullMode::Back:  return GL_BACK;
            case CullMode::None:  return GL_NONE;
        }
        spdlog::error("Unknown CullMode in TranslateCullModeToGL");
        return GL_BACK;
    }

    inline GLenum TranslateCompareOpToGL(CompareOp op)
    {
        switch (op)
        {
            case CompareOp::Less:        return GL_LESS;
            case CompareOp::LessOrEqual: return GL_LEQUAL;
            case CompareOp::Greater:     return GL_GREATER;
            case CompareOp::Always:      return GL_ALWAYS;
        }
        spdlog::error("Unknown CompareOp in TranslateCompareOpToGL");
        return GL_LESS;
    }

    struct GLVertexFormatInfo
    {
        GLint     count      = 3;
        GLenum    type       = GL_FLOAT;
        GLboolean normalized = GL_FALSE;
    };

    inline GLVertexFormatInfo TranslateVertexFormatToGL(VertexFormat format)
    {
        switch (format)
        {
            case VertexFormat::Float2: return { 2, GL_FLOAT, GL_FALSE };
            case VertexFormat::Float3: return { 3, GL_FLOAT, GL_FALSE };
            case VertexFormat::Float4: return { 4, GL_FLOAT, GL_FALSE };
        }
        spdlog::error("Unknown VertexFormat in TranslateVertexFormatToGL");
        return { 3, GL_FLOAT, GL_FALSE };
    }

    constexpr bool IsDepthFormat(TextureFormat format)
    {
        return format == TextureFormat::Depth32_FLOAT;
    }

};
