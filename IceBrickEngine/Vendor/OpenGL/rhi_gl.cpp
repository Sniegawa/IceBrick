#include "RHI/rhi.hpp"
#include "RHI/Texture.hpp"
#include "RHI/StorageBuffer.hpp"
#include "Vendor/OpenGL/StorageBuffer_gl.hpp"
#include "Vendor/OpenGL/Texture_gl.hpp"


#include <memory>
#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace IceBrick::RHI
{
    void Initialize(void* NativeWindowHandle)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(NativeWindowHandle);

        glfwMakeContextCurrent(window);

        if(!gladLoadGL(glfwGetProcAddress))
        {
            spdlog::error("Failed to initialize GLAD!");
            return;
        }

        spdlog::info("RHI initialized succesfully");

    }

    void ClearScreen(float r, float g, float b)
    {
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    std::shared_ptr<Texture> Texture::Create(const TextureDescription& description)
    {
        return std::make_shared<GLTexture>(description);
    }

    std::shared_ptr<StorageBuffer> StorageBuffer::Create(const StorageBufferDescription& description)
    {
        return std::make_shared<GLStorageBuffer>(description);
    }
};

