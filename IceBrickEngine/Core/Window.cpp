#include "Core/Window.hpp"
#include "Core/InputManager.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>


namespace IceBrick
{
    Window::Window(int width, int height, const char* title)
    {
        spdlog::info("Initializing GLFW..."); 
        if(!glfwInit())
        {
            spdlog::error("Failed to initialize GLFW!");
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_NativeHandle = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if(!m_NativeHandle)
        {
            spdlog::error("Failed to create GLFW window!");
            glfwTerminate();
            return;
        }

        glfwSetKeyCallback(m_NativeHandle, &InputManager::GlfwKeyCallback);
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_NativeHandle);
        glfwTerminate();
    }

    void Window::SwapBuffers()
    {
        glfwSwapBuffers(m_NativeHandle);
    }
    bool Window::ShouldClose()
    {
        return glfwWindowShouldClose(m_NativeHandle);
    }

    void Window::DisableCursor()
    {
        glfwSetInputMode(m_NativeHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);    
    }

    void Window::EnableCursor()
    {
        glfwSetInputMode(m_NativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
};
