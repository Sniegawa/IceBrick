#include "rhi_device_gl.hpp"

#include <array>
#include <cstdint>
#include <glad/gl.h>
#include "Core/IOHelper.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_types.hpp"
#include "Vendor/OpenGL/GLTypesHelper.hpp"
#include "spdlog/spdlog.h"

namespace IceBrick::RHI
{
    GLRenderDevice::GLRenderDevice()
    {
        glCreateFramebuffers(1, &m_DynamicFBO);
    }
    GLRenderDevice::~GLRenderDevice()
    {
        for(auto& t : m_TexturePool)
            if(t.active)
            {        
                if(t.bindlessHandle != 0)
                    glMakeTextureHandleNonResidentARB(t.bindlessHandle);
                glDeleteTextures(1,&t.glID);
            }

        for(auto& b : m_BufferPool)
            if(b.active)
                glDeleteBuffers(1,&b.glID);

        for(auto& p : m_PipelinePool)
            if(p.active)
            {
                if(p.isGraphics)
                    glDeleteVertexArrays(1, &p.vao);
                
                glDeleteProgram(p.glID);
            }

        m_TexturePool.clear();
        m_BufferPool.clear();
        m_PipelinePool.clear();

        m_FreeTextureIndices.clear();
        m_FreeBufferIndices.clear();
        m_FreePipelineIndices.clear();

        glDeleteFramebuffers(1, &m_DynamicFBO);
    }

    void GLRenderDevice::BeginFrame()
    {
        m_State.Invalidate();
    }
    void GLRenderDevice::EndFrame()
    {

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

        if(HasFlag(description.usage, TextureUsage::Bindless))
        {
            if(glGetTextureHandleARB && glMakeTextureHandleResidentARB)
            {
                node.bindlessHandle = glGetTextureHandleARB(node.glID);
                glMakeTextureHandleResidentARB(node.bindlessHandle);
            }
            else
            {
                spdlog::critical("GL_ARB_bindless_texture is not loaded in GLAD or unsupported");
            }
        }


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
            
        if(node.bindlessHandle != 0)
        {
            glMakeTextureHandleNonResidentARB(node.bindlessHandle);
            node.bindlessHandle = 0;
        }

        glDeleteTextures(1, &node.glID);
        node.glID = 0;
        node.active = false;
        node.mipLevels = 1;

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

    uint64_t GLRenderDevice::GetTextureBindlessHandle(TextureHandle handle) 
    {
        const auto& node = GetTextureNode(handle);
        if(node.bindlessHandle == 0)
            spdlog::error("Requested bindless handle for a texture not created with Bindless flag");
        return node.bindlessHandle;
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
            return;
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

    void GLRenderDevice::BindImage(uint32_t set, uint32_t binding, TextureHandle handle, ImageAccess access, int mip)
    {
        auto& node = GetTextureNode(handle);
        uint32_t flatIndex = CalculateFlatIndex(set, binding);

        GLenum glAccess = TranslateImageAccessToGL(access);
        GLenum internalFormat = TranslateInternalFormatToGL(node.format);

        glBindImageTexture(
                flatIndex,
                node.glID,
                mip,
                (node.type == TextureType::Texture3D ? GL_TRUE : GL_FALSE),
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
        node.isGraphics = false;

        PipelineHandle handle;
        handle.id = (static_cast<uint64_t>(node.generation) << 32) | slotIndex;

        return handle;
    }

    PipelineHandle GLRenderDevice::CreateGraphicsPipeline(const GraphicsPipelineDescription& description)
    {
        GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);

        std::string vertexData = ReadShaderFile(description.vertexShaderPath);
        const char* VertexSrc = vertexData.c_str();
        glShaderSource(VertexShader, 1, &VertexSrc, nullptr);
        glCompileShader(VertexShader);

        GLint success;
        glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char infoLog[512] = {0};
            glGetShaderInfoLog(VertexShader, 512, nullptr, infoLog);
            spdlog::error("Vertex shader compilation failed:\n{}",static_cast<const char*>(infoLog));
            glDeleteShader(VertexShader);
            return PipelineHandle{}; // Return invalid Handle
        }

        GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        std::string FragmentData = ReadShaderFile(description.fragmentShaderPath);
        const char* FragSrc = FragmentData.c_str();
        glShaderSource(FragmentShader,1, &FragSrc, nullptr);
        glCompileShader(FragmentShader);

        glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            char infoLog[512] = {0};
            glGetShaderInfoLog(FragmentShader, 512, nullptr, infoLog);
            spdlog::error("Fragment shader compilation failed:\n{}",static_cast<const char*>(infoLog));
            glDeleteShader(VertexShader);
            glDeleteShader(FragmentShader);
            return PipelineHandle{}; // Return invalid Handle
        }


        GLuint program = glCreateProgram();
        glAttachShader(program,VertexShader);
        glAttachShader(program,FragmentShader);
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if(!success)
        {
            char infoLog[512] = {0};
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            spdlog::error("Graphics pipeline linking failed: {}",static_cast<const char*>(infoLog));
            glDeleteShader(VertexShader);
            glDeleteShader(FragmentShader);
            glDeleteProgram(program);
            return PipelineHandle{};
        }

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        GLuint vao;
        glCreateVertexArrays(1,&vao);

        for(const auto& attr : description.vertexAttributes)
        {
            auto [count, type, normalized] = TranslateVertexFormatToGL(attr.format);
            glEnableVertexArrayAttrib(vao, attr.location);
            glVertexArrayAttribFormat(vao, attr.location, count, type, normalized, attr.offset);
            glVertexArrayAttribBinding(vao, attr.location, 0);
        }

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
        node.vao = vao;
        node.isGraphics = true;
        node.vertexStride = description.vertexStride;
        node.depthWrite = description.depthWrite;
        node.depthTest = description.depthTest;
        node.depthCompare = description.depthCompare;
        node.cullMode = description.cullMode;
        

        PipelineHandle handle;
        handle.id = (static_cast<uint64_t>(node.generation) << 32) | slotIndex;

        return handle;

    }

    void GLRenderDevice::BindGraphicsPipeline(PipelineHandle handle)
    {
        const auto& node = GetPipelineNode(handle);

        m_BoundStride = node.vertexStride;
        ApplyGraphicsState(node);
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
        if(node.isGraphics)
            glDeleteVertexArrays(1,&node.vao);
        node.isGraphics = false;

        // This ensures that no one will use object in this slot
        // while accessing it as the just deleted buffer
        node.generation++;

        m_FreePipelineIndices.push_back(index);
    }

    void GLRenderDevice::BindComputePipeline(PipelineHandle handle)
    {
        auto& node = GetPipelineNode(handle);
        if(m_State.program != node.glID)
        {
            glUseProgram(node.glID);
            m_State.program = node.glID;
        }
    }

    void GLRenderDevice::DispatchCompute(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        glDispatchCompute(groupCountX,groupCountY,groupCountZ);
    }

    void GLRenderDevice::InsertImageMemoryBarrier(MemoryBarrierBits barrier)
    {
        GLbitfield flags = TranslateMemoryBarrierToGL(barrier);
        glMemoryBarrier(flags);
    }

    void GLRenderDevice::BeginRendering(const RenderingInfo& info)
    {
        constexpr uint32_t MAX_COLOR_ATTACHMENTS = 8;
        std::array<GLenum, MAX_COLOR_ATTACHMENTS> drawBuffers = {};

        const uint32_t colorCount = std::min(static_cast<uint32_t>(info.colorAttachments.size()),MAX_COLOR_ATTACHMENTS);

        for(uint32_t i = 0; i < colorCount; i++)
        {
            const auto& att = info.colorAttachments[i];
            const auto& tex = GetTextureNode(att.texture);

            glNamedFramebufferTexture(m_DynamicFBO, GL_COLOR_ATTACHMENT0 + i, tex.glID, 0);
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        // Unbind any previous textures
        for(uint32_t i = colorCount; i < m_LastColorAttachmentCount; i++)
            glNamedFramebufferTexture(m_DynamicFBO, GL_COLOR_ATTACHMENT0 + i, 0, 0);
        m_LastColorAttachmentCount = colorCount;

        if(colorCount > 0)
        {
            glNamedFramebufferDrawBuffers(m_DynamicFBO, static_cast<GLsizei>(colorCount),drawBuffers.data());
        }
        else
        {
            GLenum none = GL_NONE;
            glNamedFramebufferDrawBuffers(m_DynamicFBO, 1, &none);
        }

        if(info.depthAttachment.has_value())
        {
            const auto& depthTex = GetTextureNode(info.depthAttachment->texture);
            glNamedFramebufferTexture(m_DynamicFBO, GL_DEPTH_ATTACHMENT, depthTex.glID, 0);
        }
        else
        {
            glNamedFramebufferTexture(m_DynamicFBO, GL_DEPTH_ATTACHMENT, 0, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, m_DynamicFBO);
        glViewport(0, 0, static_cast<GLsizei>(info.width), static_cast<GLsizei>(info.height));

        for(uint32_t i = 0; i < colorCount; i++)
        {
            const auto& att = info.colorAttachments[i];
            if(att.loadOp == LoadOp::Clear)
                glClearNamedFramebufferfv(m_DynamicFBO, GL_COLOR, static_cast<GLint>(i),&att.clearColor.x);
        }

        if(info.depthAttachment.has_value() && info.depthAttachment->loadOp == LoadOp::Clear)
        {
            // OpenGL ignores clearing depth when glDepthMask is false
            if(m_State.depthWrite != true)
            {
                glDepthMask(GL_TRUE);
                m_State.depthWrite = true;
            }
            glClearNamedFramebufferfv(m_DynamicFBO, GL_DEPTH, 0, &info.depthAttachment->clearDepth);
        }

    }
    
    void GLRenderDevice::EndRendering()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void GLRenderDevice::DrawIndexed(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexCount, uint32_t instanceCount)
    {
        if(m_State.vao == 0 || indexCount == 0 || instanceCount == 0)
            return;

        const auto& vbo = GetBufferNode(vertexBuffer);
        const auto& ibo = GetBufferNode(indexBuffer);

        glVertexArrayVertexBuffer(m_State.vao, 0, vbo.glID, 0, static_cast<GLsizei>(m_BoundStride));
        glVertexArrayElementBuffer(m_State.vao, ibo.glID);

        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indexCount),GL_UNSIGNED_INT,nullptr,static_cast<GLsizei>(instanceCount));
    }




    void GLRenderDevice::ClearScreen(float r, float g, float b)
    {
        glClearColor(r,g,b,1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }

    void GLRenderDevice::ApplyGraphicsState(const GLPipelineNode& node)
    {
        if(m_State.program != node.glID)
        {
            glUseProgram(node.glID);
            m_State.program = node.glID;
        }

        if(m_State.vao != node.vao)
        {
            glBindVertexArray(node.vao);
            m_State.vao = node.vao;
        }

        if(m_State.cullMode != node.cullMode)
        {
            if(node.cullMode == CullMode::None)
            {
                glDisable(GL_CULL_FACE);
            }
            else 
            {
                if(m_State.cullMode == CullMode::None || !m_State.cullMode.has_value())
                    glEnable(GL_CULL_FACE);
                glCullFace(node.cullMode == CullMode::Front ? GL_FRONT : GL_BACK);
            }
            m_State.cullMode = node.cullMode;
        }

        if(m_State.depthTest != node.depthTest)
        {
            node.depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
            m_State.depthTest = node.depthTest;
        }

        if(node.depthTest && m_State.depthCompare != node.depthCompare)
        {
            glDepthFunc(TranslateCompareOpToGL(node.depthCompare));
            m_State.depthCompare = node.depthCompare;
        }

        if(m_State.depthWrite != node.depthWrite)
        {
            glDepthMask(node.depthWrite ? GL_TRUE : GL_FALSE);
            m_State.depthWrite = node.depthWrite;
        }
    }

};
