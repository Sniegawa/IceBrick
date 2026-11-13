#pragma once

#include "Core/WindowInput.hpp"
#include <bitset>

struct GLFWwindow;

namespace IceBrick
{

    class InputManager
    {
    public:
        [[nodiscard]] static bool IsKeyPressed(Key key) noexcept;

        // Possibly move this or replace with API agnostic callback
        static void GlfwKeyCallback(GLFWwindow* window, int glfwKey, int scancode,int action, int mods);
    private:
        inline static std::bitset<128> s_KeyStates;
    };
};
