#include "StorageBuffer_gl.hpp"
#include "RHI/StorageBuffer.hpp"
#include "spdlog/spdlog.h"

#include <glad/gl.h>
#include <regex>

namespace IceBrick::RHI
{
    GLenum TranslateBufferUsageToGL(BufferUsage usage)
    {
        switch(usage)
        {
            case BufferUsage::Static:
                return GL_STATIC_DRAW;
            case BufferUsage::Dynamic:
                return GL_DYNAMIC_DRAW;
            case BufferUsage::Readback:
                return GL_DYNAMIC_READ;
        }
        spdlog::error("Unknown BufferUsage in TranslateBufferUsageToGL");
        return GL_STATIC_DRAW;
    }


    GLStorageBuffer::GLStorageBuffer(const StorageBufferDescription& description)
        : m_Description(description)
    {
        glCreateBuffers(1, &m_ID);
        glNamedBufferData(m_ID,m_Description.sizeInBytes,nullptr,TranslateBufferUsageToGL(description.usage));
    }

    GLStorageBuffer::~GLStorageBuffer()
    {
        glDeleteBuffers(1, &m_ID);
    }

    const StorageBufferDescription& GLStorageBuffer::GetDescription() const 
    {
        return m_Description;  
    }

    size_t GLStorageBuffer::GetSize() const
    {
        return m_Description.sizeInBytes;
    }

    void GLStorageBuffer::SetData(const void* data, size_t size, uint32_t offset)
    {
        if(size > m_Description.sizeInBytes)
            spdlog::critical("Uploading data to a buffer with not enough space. {} uploaded, {} buffer size",size,m_Description.sizeInBytes);
        glNamedBufferSubData(m_ID, offset, size, data);
    }

    void GLStorageBuffer::GetData(void* outData, size_t size, uint32_t offset) const
    {
        if(m_Description.usage != BufferUsage::Readback)
            spdlog::warn("Reading from GL buffer not marked as Readback. It may lower performance significantly!");

        glGetNamedBufferSubData(m_ID, offset, size, outData);
    }

    void* GLStorageBuffer::GetNativeHandle() const
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(m_ID));
    }

};
