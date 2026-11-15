#pragma once

#include "RHI/rhi_device.hpp"
#include "RHI/rhi_handles.hpp"
#include <cstdlib>
#include <vector>
#include "spdlog/spdlog.h"

namespace IceBrick::RHI
{

    struct GLNodeBase
    {
        unsigned int glID = 0;
        uint32_t generation = 1;
        bool active = false;
    };

    struct GLBufferNode : public GLNodeBase
    {
        uint32_t sizeInBytes = 0;
    };

    struct GLTextureNode : public GLNodeBase
    {       
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        
        TextureFormat format;
        TextureType type;

        uint32_t mipLevels = 1;
    };
    
    class GLRenderDevice final : public RenderDevice
    {
    public:
        GLRenderDevice();
        virtual ~GLRenderDevice() override;

        // FRAME LIFECYCLE
        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        // RESOURCES
        virtual BufferHandle CreateBuffer(const BufferDescription& description) override;
        virtual void DeleteBuffer(BufferHandle handle) override;

        virtual TextureHandle CreateTexture(const TextureDescription& description) override;
        virtual void DestroyTexture(TextureHandle handle) override;
        virtual void* GetTextureNativeHandle(TextureHandle handle) override;

        // Data transfer
        virtual void SetBufferData(BufferHandle handle, const void* data, uint32_t size, uint32_t offset = 0) override;
        virtual void SetTextureData(TextureHandle handle, const void* data, uint32_t size) override;

        virtual void GetBufferData(BufferHandle handle, void* outData, uint32_t size, uint32_t offset = 0) override;

        virtual void ClearScreen(float r, float g, float b) override;


    private:
        std::vector<GLBufferNode> m_BufferPool;
        std::vector<uint32_t> m_FreeBufferIndices;

        std::vector<GLTextureNode> m_TexturePool;
        std::vector<uint32_t> m_FreeTextureIndices;

    private:
        inline GLBufferNode& GetBufferNode(BufferHandle handle)
        {
            uint32_t index = handle.GetIndex();
        #ifndef NDEBUG // Macro for optimising out checks on release
            if(
                index >= m_BufferPool.size() || 
                m_BufferPool[index].generation != handle.GetGeneration() || 
                !m_BufferPool[index].active)
            {
                spdlog::critical("Attempted to get an invalid or stale BufferHandle");
                std::abort();
            }
        #endif
            return m_BufferPool[index];
        }

        inline GLTextureNode& GetTextureNode(TextureHandle handle)
        {
            uint32_t index = handle.GetIndex();
        #ifndef NDEBUG
            if(
                index >= m_TexturePool.size() || 
                m_TexturePool[index].generation != handle.GetGeneration() ||
                !m_TexturePool[index].active)
            {
                spdlog::warn("Attempted to get an invalid or stale TextureHandle");
                std::abort();
            }

        #endif
            return m_TexturePool[index];
        }
    };
};
