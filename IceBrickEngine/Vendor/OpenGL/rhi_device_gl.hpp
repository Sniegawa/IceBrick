#pragma once

#include "RHI/rhi_device.hpp"
#include "RHI/rhi_handles.hpp"
#include <cstdlib>
#include <optional>
#include <vector>
#include "RHI/rhi_types.hpp"
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
        size_t sizeInBytes = 0;
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

    struct GLPipelineNode : public GLNodeBase
    {
        bool isGraphics = false;
        
        unsigned int vao = 0;
        uint32_t vertexStride = 0;

        CullMode cullMode = CullMode::Back;
        bool depthTest = true;
        bool depthWrite = true;
        CompareOp depthCompare = CompareOp::Less;
    };

    struct GLStateCache
    {
        uint32_t program = 0;
        uint32_t vao = 0;
        std::optional<CullMode> cullMode;
        std::optional<bool> depthTest;
        std::optional<bool> depthWrite;
        std::optional<CompareOp> depthCompare;

        void Invalidate() { *this = {}; }
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
        virtual void SetBufferData(BufferHandle handle, const void* data, size_t size, uint32_t offset = 0) override;
        virtual void SetTextureData(TextureHandle handle, const void* data, size_t size, int mip = 0) override;

        virtual void GetBufferData(BufferHandle handle, void* outData, size_t size, uint32_t offset = 0) override;
    
        // Data Binding
        virtual void BindUniformBuffer(uint32_t set, uint32_t binding, BufferHandle handle)override;
        virtual void BindStorageBuffer(uint32_t set, uint32_t binding, BufferHandle handle)override;

        virtual void BindTexture(uint32_t set, uint32_t binding, TextureHandle handle)override;
        virtual void BindImage(uint32_t set, uint32_t binding, TextureHandle handle, ImageAccess access, int mip = 0) override;

        // Pipelines
        virtual PipelineHandle CreateComputePipeline(std::string_view computeShaderSource) override;
        virtual PipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDescription& description)override;
        virtual void DestroyPipeline(PipelineHandle handle) override;

        virtual void BindComputePipeline(PipelineHandle handle) override;
        virtual void BindGraphicsPipeline(PipelineHandle handle)override;

        // Drawing
        
        virtual void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        virtual void InsertImageMemoryBarrier(MemoryBarrierBits barrier) override;

        virtual void BeginRendering(const RenderingInfo& info)override;
        virtual void EndRendering()override;

        virtual void DrawIndexed(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexCount, uint32_t instanceCount = 1)override;




        virtual void ClearScreen(float r, float g, float b) override;


    private:
        std::vector<GLBufferNode> m_BufferPool;
        std::vector<uint32_t> m_FreeBufferIndices;

        std::vector<GLTextureNode> m_TexturePool;
        std::vector<uint32_t> m_FreeTextureIndices;

        std::vector<GLPipelineNode> m_PipelinePool;
        std::vector<uint32_t> m_FreePipelineIndices;

        GLStateCache m_State;

        unsigned int m_DynamicFBO = 0;
        uint32_t m_BoundStride = 0;
        uint32_t m_LastColorAttachmentCount = 0;
    private:

        void ApplyGraphicsState(const GLPipelineNode& node);

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

        inline GLPipelineNode& GetPipelineNode(PipelineHandle handle)
        {
            uint32_t index = handle.GetIndex();
        #ifndef NDEBUG
            if(
                index >= m_PipelinePool.size() || 
                m_PipelinePool[index].generation != handle.GetGeneration() ||
                !m_PipelinePool[index].active)
            {
                spdlog::warn("Attempted to get an invalid or stale PipelineHandle");
                std::abort();
            }

        #endif
            return m_PipelinePool[index];
        }
    };
};
