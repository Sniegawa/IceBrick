#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace IceBrick::RHI
{
    enum class BufferUsage : uint8_t
    {
        Static, // CPU writes once, GPU reads/writes often
        Dynamic, // CPU writes often, GPU reads
        Readback // GPU -> CPU
    };

    struct StorageBufferDescription
    {
        size_t sizeInBytes = 0;
        BufferUsage usage = BufferUsage::Static;
    };

    class StorageBuffer
    {
    public:
        virtual ~StorageBuffer() = default;

        virtual const StorageBufferDescription& GetDescription() const = 0;
        virtual size_t GetSize() const = 0;

        virtual void SetData(const void* data, size_t size, uint32_t offset = 0) = 0;
        virtual void GetData(void* outData, size_t size, uint32_t offset = 0) const = 0;

        virtual void* GetNativeHandle() const = 0;

        static std::shared_ptr<StorageBuffer> Create(const StorageBufferDescription& description);
    };

};
