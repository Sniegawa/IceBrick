#include "rhi_device_gl.hpp"

#include <cstdint>
#include <glad/gl.h>
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_types.hpp"
#include "Vendor/OpenGL/TextureHelper_gl.hpp"
#include "spdlog/spdlog.h"

namespace IceBrick::RHI
{
    GLRenderDevice::GLRenderDevice()
    {

    }
    GLRenderDevice::~GLRenderDevice()
    {
        for(auto& t : m_TexturePool)
            if(t.active)
                glDeleteTextures(1,&t.glID);

        for(auto& b : m_BufferPool)
            if(b.active)
                glDeleteBuffers(1,&b.glID);

        for(auto& p : m_PipelinePool)
            if(p.active)
                glDeleteProgram(p.glID);

        m_TexturePool.clear();
        m_BufferPool.clear();
        m_PipelinePool.clear();

        m_FreeTextureIndices.clear();
        m_FreeBufferIndices.clear();
        m_FreePipelineIndices.clear();
    }

    void GLRenderDevice::BeginFrame()
    {

    }
    void GLRenderDevice::EndFrame()
    {

    }

    GLbitfield TranslateMemoryAccessToGLFlags(BufferMemoryAccess access)
    {
        GLbitfield flags = 0;

        switch(access)
        {
        case BufferMemoryAccess::GpuOnly:
            flags = GL_DYNAMIC_STORAGE_BIT;
            break;
        case BufferMemoryAccess::CpuToGpu:
            flags = GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            break;
        case BufferMemoryAccess::GpuToCpu:
            flags = GL_CLIENT_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
            break;
        }

        return flags;
    }

    BufferHandle GLRenderDevice::CreateBuffer(const BufferDescription& description)
    {
        uint32_t slotIndex = 0;

        if(!m_FreeBufferIndices.empty())
        {
            slotIndex = m_FreeBufferIndices.back();
            m_FreeBufferIndices.pop_back();
        }
        else 
        {
            slotIndex = static_cast<uint32_t>(m_BufferPool.size());
            m_BufferPool.emplace_back();
        }

        GLBufferNode& node = m_BufferPool[slotIndex];
        node.active = true;
        node.sizeInBytes = description.sizeInBytes;

        glCreateBuffers(1, &node.glID);

        auto flags = TranslateMemoryAccessToGLFlags(description.memoryAccess);

        glNamedBufferStorage(node.glID, description.sizeInBytes, nullptr, flags);

        BufferHandle handle;
        handle.id = (static_cast<uint64_t>(node.generation) << 32) | slotIndex;

        return handle;
    }

    void GLRenderDevice::DeleteBuffer(BufferHandle handle)
    {
        uint32_t index = handle.GetIndex();
        uint32_t handleGen = handle.GetGeneration();

        if(index >= m_BufferPool.size() || m_BufferPool[index].generation != handleGen || !m_BufferPool[index].active)
        {
            spdlog::warn("Attempted to destroy an invalid or stale BufferHandle");
            return;
        }

        GLBufferNode& node = m_BufferPool[index];

        glDeleteBuffers(1, &node.glID);
        node.glID = 0;
        node.sizeInBytes = 0;
        node.active = false;

        // This ensures that no one will use object in this slot
        // while accessing it as the just deleted buffer
        node.generation++;

        m_FreeBufferIndices.push_back(index);

    }

    TextureHandle GLRenderDevice::CreateTexture(const TextureDescription& description)
    {
        uint32_t slotIndex = 0;
        
        if(!m_FreeTextureIndices.empty())
        {
            slotIndex = m_FreeTextureIndices.back();
            m_FreeTextureIndices.pop_back();
        }
        else 
        {
            slotIndex = static_cast<uint32_t>(m_TexturePool.size());
            m_TexturePool.emplace_back();
        }

        GLTextureNode& node = m_TexturePool[slotIndex];
        node.width = description.width;
        node.height = description.height;
        node.depth = description.depth;
        node.format = description.format;
        node.type = description.type;
        node.active = true;
        if(description.generateMips)
            node.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(description.width, description.height)))) + 1;

        GLenum internalFormat = TranslateInternalFormatToGL(description.format);

        if(description.type == TextureType::Texture2D)
        {
            glCreateTextures(GL_TEXTURE_2D, 1, &node.glID);
            glTextureStorage2D(node.glID, node.mipLevels, internalFormat, description.width, description.height);
        }
        else if(description.type == TextureType::Texture3D)
        {
            glCreateTextures(GL_TEXTURE_3D, 1, &node.glID);
            glTextureStorage3D(node.glID, node.mipLevels, internalFormat, description.width, description.height, description.depth);
        }

        GLenum wrap = TranslateWrapToGL(description.wrap);
        
        glTextureParameteri(node.glID, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(node.glID, GL_TEXTURE_WRAP_T, wrap);
        if(description.type == TextureType::Texture3D)
            glTextureParameteri(node.glID, GL_TEXTURE_WRAP_R, wrap);

        glTextureParameteri(node.glID, GL_TEXTURE_MIN_FILTER, TranslateFilterToGL(description.filter, description.generateMips)); 
        glTextureParameteri(node.glID, GL_TEXTURE_MAG_FILTER, TranslateFilterToGL(description.filter, false));

   


        TextureHandle handle;
        handle.id = (static_cast<uint64_t>(node.generation) << 32) | slotIndex;

        return handle;
    }

    void GLRenderDevice::DestroyTexture(TextureHandle handle)
    {
        uint32_t index = handle.GetIndex();
        uint32_t handleGen = handle.GetGeneration();

        if(index >= m_TexturePool.size() || m_TexturePool[index].generation != handleGen || !m_TexturePool[index].active)
        {
            spdlog::warn("Attempted to destroy an invalid or stale TextureHandle");
            return;
        }

        GLTextureNode& node = m_TexturePool[index];

        glDeleteTextures(1, &node.glID);
        node.glID = 0;
        node.active = false;

        // This ensures that no one will use object in this slot
        // while accessing it as the just deleted buffer
        node.generation++;

        m_FreeTextureIndices.push_back(index);
    }

    void* GLRenderDevice::GetTextureNativeHandle(TextureHandle handle)
    {
        auto& node = GetTextureNode(handle);
        return reinterpret_cast<void*>(static_cast<uintptr_t>(node.glID));
    }

    void GLRenderDevice::SetBufferData(BufferHandle handle, const void* data, size_t size, uint32_t offset)
    {
        GLBufferNode& node = GetBufferNode(handle);
        if(size > node.sizeInBytes)
        {
            spdlog::critical("Uploading data to a buffer with not enough space. {} uploaded, {} buffer size",size,node.sizeInBytes);
            return;
        }
        glNamedBufferSubData(node.glID, offset, size, data);
    }

    void GLRenderDevice::SetTextureData(TextureHandle handle, const void* data, size_t size, int mip)
    {
        GLTextureNode& node = GetTextureNode(handle);
        
        if(mip < 0 || static_cast<uint32_t>(mip) >= node.mipLevels)
        {
            spdlog::error("Invalid mip level {} (allocated levels: {})", mip, node.mipLevels);
        }

        size_t mipWidth = std::max(1u, node.width >> mip);
        size_t mipHeight = std::max(1u, node.height >> mip);
        size_t mipDepth = std::max(1u, node.depth >> mip);

        size_t expectedSize = mipWidth * mipHeight * mipDepth * GetBytesPerPixel(node.format);
        if(size != expectedSize)
        {
            spdlog::error("Texture upload bounds mismatch! Format requires {} bytes, but {} bytes were passed",expectedSize, size);
            return;
        }


        GLenum format = TranslateFormatToGL(node.format);
        GLenum type = TranslateTypeToGL(node.format);

        if (node.format == TextureFormat::R16_UINT)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        else
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
         

        if(node.type == TextureType::Texture2D)
        {
            glTextureSubImage2D
                (
                    node.glID,
                    mip,
                    0,
                    0,
                    mipWidth,
                    mipHeight,
                    format,
                    type,
                    data
                );
        }
        else if(node.type == TextureType::Texture3D)
        {
            glTextureSubImage3D
                (
                    node.glID,
                    mip,
                    0,
                    0,
                    0,
                    mipWidth,
                    mipHeight,
                    mipDepth,
                    format,
                    type,
                    data
                );
        }
    }

    void GLRenderDevice::GetBufferData(BufferHandle handle, void* outData, size_t size, uint32_t offset)
    {
        GLBufferNode& node = GetBufferNode(handle);

        glGetNamedBufferSubData(node.glID, offset, size, outData);
    }

    // TODO : MOVE SOMEWHERE ELSE
    GLenum TranslateImageAccessToGL(ImageAccess access)
    {
        switch(access) {
            case ImageAccess::ReadOnly: return GL_READ_ONLY;
            case ImageAccess::WriteOnly: return GL_WRITE_ONLY;
            case ImageAccess::ReadWrite: return GL_READ_WRITE;
        }
        spdlog::warn("Unknown ImageAccess in TranslateImageAccessToGL");
        return GL_READ_WRITE;
    }

    constexpr uint32_t MAX_BINDINGS_PER_SET = 16;

    constexpr uint32_t CalculateFlatIndex(uint32_t set, uint32_t binding)
    {
        return (set * MAX_BINDINGS_PER_SET) + binding;
    }

    void GLRenderDevice::BindUniformBuffer(uint32_t set, uint32_t binding, BufferHandle handle)
    {
        auto& node = GetBufferNode(handle);
        uint32_t flatIndex = CalculateFlatIndex(set, binding); 

        glBindBufferBase(GL_UNIFORM_BUFFER,flatIndex,node.glID);
    }

    void GLRenderDevice::BindStorageBuffer(uint32_t set, uint32_t binding, BufferHandle handle)
    {
        auto& node = GetBufferNode(handle);
        uint32_t flatIndex = CalculateFlatIndex(set, binding);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, flatIndex, node.glID);
    }

    void GLRenderDevice::BindTexture(uint32_t set, uint32_t binding, TextureHandle handle)
    {
        auto& node = GetTextureNode(handle);
        uint32_t flatIndex = CalculateFlatIndex(set, binding);

        glBindTextureUnit(flatIndex, node.glID);
    }

    void GLRenderDevice::BindImage(uint32_t set, uint32_t binding, TextureHandle handle, ImageAccess access)
    {
        auto& node = GetTextureNode(handle);
        uint32_t flatIndex = CalculateFlatIndex(set, binding);

        GLenum glAccess = TranslateImageAccessToGL(access);
        GLenum internalFormat = TranslateInternalFormatToGL(node.format);

        glBindImageTexture(
                flatIndex,
                node.glID,
                0,
                (node.type == TextureType::Texture2D ? GL_FALSE : GL_TRUE),
                0,
                glAccess,
                internalFormat
        );
    }


    PipelineHandle GLRenderDevice::CreateComputePipeline(std::string_view computeShaderSource)
    {
        GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
        const char* src = computeShaderSource.data();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char infoLog[512] = {0};
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            spdlog::error("Compute Shader Compilation Failed:\n{}",static_cast<const char*>(infoLog));
            glDeleteShader(shader);
            return PipelineHandle{}; // Return invalid Handle
        }

        GLuint program = glCreateProgram();
        glAttachShader(program,shader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success)
        {
            char infoLog[512] = {0};
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            spdlog::error("Compute Shader Linking failed: {}",static_cast<const char*>(infoLog));
            return PipelineHandle{};
        }

        glDeleteShader(shader);

        uint32_t slotIndex = 0;
        if(!m_FreePipelineIndices.empty())
        {
            slotIndex = m_FreePipelineIndices.back();
            m_FreePipelineIndices.pop_back();
        }
        else 
        {
            slotIndex = static_cast<uint32_t>(m_PipelinePool.size());
            m_PipelinePool.emplace_back();
        }

        auto& node = m_PipelinePool[slotIndex];
        node.active = true;
        node.glID = program;

        PipelineHandle handle;
        handle.id = (static_cast<uint64_t>(node.generation) << 32) | slotIndex;

        return handle;
    }

    void GLRenderDevice::DestroyPipeline(PipelineHandle handle)
    {
        uint32_t index = handle.GetIndex();
        uint32_t handleGen = handle.GetGeneration();

        if(index >= m_PipelinePool.size() || m_PipelinePool[index].generation != handleGen || !m_PipelinePool[index].active)
        {
            spdlog::warn("Attempted to destroy an invalid or stale PipelineHandle");
            return;
        }

        GLPipelineNode& node = m_PipelinePool[index];

        glDeleteProgram(node.glID);
        node.glID = 0;
        node.active = false;

        // This ensures that no one will use object in this slot
        // while accessing it as the just deleted buffer
        node.generation++;

        m_FreePipelineIndices.push_back(index);
    }

    void GLRenderDevice::BindComputePipeline(PipelineHandle handle)
    {
        auto& node = GetPipelineNode(handle);
        glUseProgram(node.glID);
    }

    void GLRenderDevice::DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        glDispatchCompute(groupCountX,groupCountY,groupCountZ);
    }

    void GLRenderDevice::InsertImageMemoryBarrier()
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    }



    void GLRenderDevice::ClearScreen(float r, float g, float b)
    {
        glClearColor(r,g,b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }

};
