#include "RHI/rhi.hpp"
#include "RHI/rhi_device.hpp"
#include "Vendor/OpenGL/rhi_device_gl.hpp"

#include <cassert>
#include <memory>
#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace IceBrick::RHI
{
    static std::unique_ptr<RenderDevice> g_Device = nullptr;

    RenderDevice* InitializeDevice(void* NativeWindowHandle)
    {
        if(g_Device)
        {
            spdlog::error("RHI is already intialized");
            return g_Device.get();
        }

        GLFWwindow* window = static_cast<GLFWwindow*>(NativeWindowHandle);

        glfwMakeContextCurrent(window);

        if(!gladLoadGL(glfwGetProcAddress))
        {
            spdlog::error("Failed to initialize GLAD!");
            return nullptr;
        }

        spdlog::info("RHI initialized succesfully");

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback([](
                    GLenum source,
                    GLenum type,
                    GLuint id,
                    GLenum severity,
                    GLsizei length,
                    const GLchar* message,
                    const void* userParam)
                    {
                        if(severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;
                        if(severity == GL_DEBUG_SEVERITY_HIGH)
                            spdlog::critical("OpenGL ERROR: {}", static_cast<const char*>(message));
                        else
                            spdlog::warn("OpenGL WARNING: {}", static_cast<const char*>(message));
                    },nullptr);

        g_Device = std::make_unique<GLRenderDevice>();

        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* version  = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        spdlog::info("Running OpenGL {}, on {}",version, renderer);

        return g_Device.get(); 
    }
    
    RenderDevice* GetDevice()
    {
        assert(g_Device != nullptr);
        return g_Device.get();
    }

    void ShutdownDevice()
    {
        if(!g_Device)
        {
            spdlog::error("Tried to shutdown a device that doesnt exist");
            return;
        }
        g_Device.reset();
    }
};
