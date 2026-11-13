#include "InputManager.hpp"
#include "Core/WindowInput.hpp"

#include <GLFW/glfw3.h>

namespace IceBrick
{
    bool InputManager::IsKeyPressed(Key key) noexcept
    {
        if(key == Key::Unknown) return false;
        return s_KeyStates[static_cast<size_t>(key)];
    }

    void InputManager::GlfwKeyCallback(GLFWwindow* window, int glfwKey, int scancode,int action, int mods)
    {
        if(glfwKey == GLFW_KEY_UNKNOWN) return;

        Key key = MapGlfwKey(glfwKey);
        if(key == Key::Unknown) return;

        if(action == GLFW_PRESS)
            s_KeyStates.set(static_cast<uint32_t>(key));
        else if(action == GLFW_RELEASE)
            s_KeyStates.reset(static_cast<uint32_t>(key));
    }

};
