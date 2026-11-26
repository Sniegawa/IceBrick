#pragma once

#include "RHI/rhi_types.hpp"
#include "RHI/rhi_handles.hpp"
#include <span>
#include <string_view>
#include <ranges>
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
        virtual void SetBufferData(BufferHandle handle, const void* data, size_t size, uint32_t offset = 0) = 0;
        virtual void SetTextureData(TextureHandle handle, const void* data, size_t size, int mip = 0) = 0;

        virtual void GetBufferData(BufferHandle handle, void* outData, size_t size, uint32_t offset = 0) = 0;

        // Data Binding
        virtual void BindUniformBuffer(uint32_t set, uint32_t binding, BufferHandle handle) = 0;
        virtual void BindStorageBuffer(uint32_t set, uint32_t binding, BufferHandle handle) = 0;

        virtual void BindTexture(uint32_t set, uint32_t binding, TextureHandle handle) = 0;
        virtual void BindImage(uint32_t set, uint32_t binding, TextureHandle handle, ImageAccess access, int mip = 0) = 0;

        // Pipelines
        virtual PipelineHandle CreateComputePipeline(std::string_view computeShaderSource) = 0;
        virtual void DestroyPipeline(PipelineHandle handle) = 0;
        virtual void BindComputePipeline(PipelineHandle handle) = 0;
        virtual void DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void InsertImageMemoryBarrier(MemoryBarrierBits barrier) = 0;


        virtual void ClearScreen(float r, float g, float b) = 0;

        template<typename T>
        void Upload(BufferHandle handle, std::span<const T> data, uint32_t byteOffset = 0)
        {
            SetBufferData(handle, data.data(), data.size_bytes());
        }

        template<typename T>
        requires (!std::ranges::range<T>)
        void Upload(BufferHandle handle, const T& data, uint32_t byteOffset = 0)
        {
            SetBufferData(handle, &data, sizeof(T));
        }

        template<typename T>
        void Download(BufferHandle handle, std::span<T> outData, uint32_t byteOffset = 0)
        {
            GetBufferData(handle, outData.data(), static_cast<uint32_t>(outData.size_bytes()), byteOffset);
        }
    };
};
