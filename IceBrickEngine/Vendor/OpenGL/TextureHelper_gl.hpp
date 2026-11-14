#pragma once

#include "Renderer/TextureDescription.hpp"
#include <glad/gl.h>
#include <spdlog/spdlog.h>

namespace IceBrick::RHI
{
    inline GLenum TranslateInternalFormatToGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8_UNORM:   return GL_RGBA8;
            case TextureFormat::RGBA16_FLOAT:  return GL_RGBA16F;
            case TextureFormat::RGBA32_FLOAT:  return GL_RGBA32F;
            case TextureFormat::R32_UINT:      return GL_R32UI;
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
            case TextureFormat::Depth32_FLOAT: return GL_DEPTH_COMPONENT;
        }
        spdlog::error("Unknown TextureFormat in TranslateFormatToGL");
        return GL_RGBA;
    }

    inline GLenum TranslateTypeToGL(TextureFormat format) {
        switch (format) {
            case TextureFormat::RGBA8_UNORM:   return GL_UNSIGNED_BYTE;
            case TextureFormat::RGBA16_FLOAT:  return GL_FLOAT; 
            case TextureFormat::RGBA32_FLOAT:  return GL_FLOAT;
            case TextureFormat::R32_UINT:      return GL_UNSIGNED_INT;
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
};
