#pragma once

#include "RHI/rhi_handles.hpp"
#include "glm/ext/vector_float4.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace IceBrick::RHI
{
    enum class BufferUsage : uint32_t
    {
        UniformBuffer = 1 << 0,
        StorageBuffer = 1 << 1,
        TransferSrc = 1 << 2,
        TransferDst = 1 << 3,
        IndexBuffer = 1 << 4,
        VertexBuffer = 1 << 5,
    };

    inline BufferUsage operator|(BufferUsage a, BufferUsage b)
    {
        return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    enum class BufferMemoryAccess : uint8_t
    {
        // No read or write from cpu directly
        GpuOnly,
        // Dynamic cpu to gpu usefull for writing every frame
        CpuToGpu,
        // Readback
        GpuToCpu
    };

    struct BufferDescription
    {
        size_t sizeInBytes = 0;
        BufferUsage usage = BufferUsage::StorageBuffer;
        BufferMemoryAccess memoryAccess = BufferMemoryAccess::CpuToGpu;
    };


    // ============ TEXTURE ==============
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
        R16_UINT,
        Depth32_FLOAT
    };

    enum class TextureType
    {
        Texture2D,
        Texture3D
    };

    enum class TextureFilter
    {
        Nearest,
        Linear
    };

    enum class TextureWrap
    {
        Repeat,
        ClampToEdge
    };

    struct TextureDescription
    {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        TextureType type = TextureType::Texture2D;
        TextureFormat format = TextureFormat::RGBA8_UNORM;
        TextureUsage usage = TextureUsage::Sampled;
        TextureFilter filter = TextureFilter::Linear;
        TextureWrap wrap = TextureWrap::Repeat;
        bool generateMips = false;
    };

    enum class ImageAccess : uint8_t
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    enum class MemoryBarrierBits : uint32_t
    {
        ShaderImageAccess = 1 << 0,
        TextureFetch = 1 << 1,
        ShaderStorage = 1 << 2
    };

    inline MemoryBarrierBits operator|(MemoryBarrierBits a, MemoryBarrierBits b)
    {
        return static_cast<MemoryBarrierBits>(
                static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
        );
    }

    // Graphics pipeline
    enum class CullMode : uint8_t { None, Front, Back };
    enum class CompareOp : uint8_t { Less, LessOrEqual, Greater, Always };
    enum class LoadOp : uint8_t { Load, Clear, DontCare };
    enum class StoreOp : uint8_t { Store, DontCare };

    enum class VertexFormat : uint8_t { Float2, Float3, Float4 };
    
    struct VertexAttribute
    {
        uint32_t location = 0;
        VertexFormat format = VertexFormat::Float3;
        uint32_t offset = 0;
    };

    struct GraphicsPipelineDescription
    {
        std::string_view vertexShaderPath;
        std::string_view fragmentShaderPath;

        std::span<const VertexAttribute> vertexAttributes;
        uint32_t vertexStride = 0;

        // Ignored by OpenGL
        std::span<const TextureFormat> colorFormats;
        TextureFormat depthFormat = TextureFormat::Depth32_FLOAT;

        CullMode cullMode = CullMode::Back;
        bool depthTest = true;
        bool depthWrite = true;
        CompareOp depthCompare = CompareOp::Less;
    };

    struct RenderAttachment
    {
        TextureHandle texture = {};
        LoadOp loadOp = LoadOp::Load;
        StoreOp storeOp = StoreOp::Store;
        glm::vec4 clearColor = glm::vec4(0.0f,0.0f,0.0f,1.0f);
        float clearDepth = 1.0f;
    };

    struct RenderingInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        std::span<const RenderAttachment> colorAttachments;
        std::optional<RenderAttachment> depthAttachment;
    };

};
