#pragma once

struct GLFWwindow;



namespace IceBrick
{
    


    class Window
    {
    public:
        Window(int width, int height, const char* title);
        ~Window();

        // No copy constructor
        Window(const Window&) = delete;
        // No copy assignment operator
        Window& operator=(const Window&) = delete;

        // No move constructor
        Window(Window&& other) noexcept;
        // No move assignment operator
        Window& operator=(Window&& other) noexcept;


        void SwapBuffers();

        void* GetNativeHandle() { return static_cast<void*>(m_NativeHandle); }

        bool ShouldClose();

    private:
        GLFWwindow* m_NativeHandle;
    };
};
