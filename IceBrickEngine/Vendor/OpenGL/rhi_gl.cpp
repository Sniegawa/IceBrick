#include <Renderer/rhi.hpp>

#include <spdlog/spdlog.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace IceBrick::RHI
{

GLFWwindow* InitWindow(int width, int height, const char* title)
{
    spdlog::info("Initializing GLFW..."); 
    if(!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW!");
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if(!window)
    {
        spdlog::error("Failed to create GLFW window!");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGL(glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD!");
        return nullptr;
    }

    spdlog::info("Window initialized succesfully");
    return window;
}

void ClearScreen(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Shutdown(GLFWwindow* window)
{
    spdlog::info("Shutting down engine...");
    glfwDestroyWindow(window);
    glfwTerminate();
}

};

