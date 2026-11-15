#pragma once

#include "RHI/rhi_types.hpp"
#include "RHI/rhi_handles.hpp"
#include <string_view>
namespace IceBrick::RHI
{
    class RenderDevice
    {
    public:
        virtual ~RenderDevice() = default;

        // FRAME LIFECYCLE
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        // RESOURCES
        virtual BufferHandle CreateBuffer(const BufferDescription& description) = 0;
        virtual void DeleteBuffer(BufferHandle handle) = 0;

        virtual TextureHandle CreateTexture(const TextureDescription& desc) = 0;
        virtual void DestroyTexture(TextureHandle handle) = 0;
        virtual void* GetTextureNativeHandle(TextureHandle handle) = 0;

        // Data transfer
        virtual void SetBufferData(BufferHandle handle, const void* data, uint32_t size, uint32_t offset = 0) = 0;
        virtual void SetTextureData(TextureHandle handle, const void* data, uint32_t size) = 0;

        virtual void GetBufferData(BufferHandle handle, void* outData, uint32_t size, uint32_t offset = 0) = 0;

        // Data Binding
        virtual void BindUniformBuffer(uint32_t set, uint32_t binding, BufferHandle handle) = 0;
        virtual void BindStorageBuffer(uint32_t set, uint32_t binding, BufferHandle handle) = 0;

        virtual void BindTexture(uint32_t set, uint32_t binding, TextureHandle handle) = 0;
        virtual void BindImage(uint32_t set, uint32_t binding, TextureHandle handle, ImageAccess access) = 0;

        // Pipelines
        virtual PipelineHandle CreateComputePipeline(std::string_view computeShaderSource) = 0;

        virtual void BindComputePipeline(PipelineHandle handle) = 0;
        virtual void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void InsertImageMemoryBarrier() = 0;


        virtual void ClearScreen(float r, float g, float b) = 0;
    };
};
