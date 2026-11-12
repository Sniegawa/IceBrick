#pragma once

struct GLFWwindow;

namespace IceBrick::RHI
{        
    GLFWwindow* InitWindow(int width, int height, const char* title);

    void ClearScreen(float r, float g, float b);

    void Shutdown(GLFWwindow* window);
}
