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


        g_Device = std::make_unique<GLRenderDevice>();

        return g_Device.get(); 
    }
    
    RenderDevice* GetDevice()
    {
        assert(g_Device != nullptr);
        return g_Device.get();
    }
};
