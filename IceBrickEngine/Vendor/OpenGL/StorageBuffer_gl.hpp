#pragma once

#include "RHI/StorageBuffer.hpp"

namespace IceBrick::RHI
{

    class GLStorageBuffer : public StorageBuffer
    {
    public:
        GLStorageBuffer(const StorageBufferDescription& description);
        virtual ~GLStorageBuffer() override;

        virtual const StorageBufferDescription& GetDescription() const override;
        virtual size_t GetSize() const override;

        virtual void SetData(const void* data, size_t size, uint32_t offset = 0) override;
        virtual void GetData(void* outData, size_t size, uint32_t offset = 0) const override;

        virtual void* GetNativeHandle() const override;

    private:
        unsigned int m_ID;

        StorageBufferDescription m_Description;
    };

};
