#include <iostream>
#include "Core/InputManager.hpp"
#include "Core/WindowInput.hpp"
#include "Renderer/rhi.hpp"
#include "Core/Window.hpp"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>


int main(int argc, char** argv)
{
    auto window = IceBrick::Window(1280, 720, "IceBrick");
    
    IceBrick::RHI::Initialize(window.GetNativeHandle());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window.GetNativeHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    while(!window.ShouldClose())
    {
        glfwPollEvents();
       
        if(IceBrick::InputManager::IsKeyPressed(IceBrick::Key::Space))
            spdlog::info("SPACE");
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

        if(ImGui::Begin("Test"))
        {
            ImGui::Text("TEST");
        }
        ImGui::End();

        ImGui::Render();
        IceBrick::RHI::ClearScreen(0.1f, 0.11f, 0.12f);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.SwapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
