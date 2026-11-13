#pragma once

#include <cstdint>

namespace IceBrick::RHI
{
    enum class TextureUsage : uint8_t
    {
        None = 0,
        Sampled = 1 << 0,           // Read in shader via sampler2D/sampler3D
        Storage = 1 << 1,           // Read/Write in shader via image2D/image3D
        ColorAttachment = 1 << 2,   // Used as a Framebuffer output
        DepthAttachment = 1 << 3    // Used as a Depth buffer
    };

    inline TextureUsage operator|(TextureUsage a, TextureUsage b)
    {
        return static_cast<TextureUsage>(
                static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
        );
    }

    inline bool operator&(TextureUsage a, TextureUsage b)
    {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    // Obviously theres more
    enum class TextureFormat
    {
        RGBA8_UNORM,
        RGBA16_FLOAT,
        RGBA32_FLOAT,
        R32_UINT,
        Depth32_FLOAT
    };

    enum class TextureType
    {
        Texture2D,
        Texture3D
    };

    struct TextureDescription
    {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        TextureType type = TextureType::Texture2D;
        TextureFormat format = TextureFormat::RGBA8_UNORM;
        TextureUsage usage = TextureUsage::Sampled;
        bool generateMips = false;
    };

    enum class ImageAccess : uint8_t
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };
};
