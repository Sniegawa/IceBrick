#include "Core/InputManager.hpp"
#include "Core/WindowInput.hpp"
#include "Core/Window.hpp"
#include "Renderer/rhi.hpp"
#include "Renderer/TextureDescription.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/StorageBuffer.hpp"

#include <cstdint>
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

    IceBrick::RHI::TextureDescription texDescription;
    texDescription.usage = IceBrick::RHI::TextureUsage::Sampled;
    texDescription.width = 4;
    texDescription.height = 4;
    texDescription.type = IceBrick::RHI::TextureType::Texture2D;
    texDescription.format = IceBrick::RHI::TextureFormat::RGBA8_UNORM;
    texDescription.filter = IceBrick::RHI::TextureFilter::Nearest; // Imgui ignores this
    texDescription.wrap = IceBrick::RHI::TextureWrap::Repeat;
    texDescription.generateMips = false;

    uint32_t TEXDATA[16] = { 
    0xFF0000FF, 0xFF00FF00, 0xFF0000FF, 0xFF00FF00,
    0xFF00FF00, 0xFF0000FF, 0xFF00FF00, 0xFF0000FF,
    0xFF0000FF, 0xFF00FF00, 0xFF0000FF, 0xFF00FF00,
    0xFF00FF00, 0xFF0000FF, 0xFF00FF00, 0xFF0000FF
};    auto tex = IceBrick::RHI::Texture::Create(texDescription);
    tex->SetData(TEXDATA, sizeof(TEXDATA) );

    unsigned int TextureHandle = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(tex->GetNativeHandle()));

    IceBrick::RHI::StorageBufferDescription bufDescription;
    bufDescription.sizeInBytes = sizeof(TEXDATA); // - 1; // To test critical warning
    bufDescription.usage = IceBrick::RHI::BufferUsage::Static;

    auto Buffer = IceBrick::RHI::StorageBuffer::Create(bufDescription);

    Buffer->SetData(TEXDATA, sizeof(TEXDATA));


    uint32_t ReadData;

    Buffer->GetData(&ReadData, sizeof(uint32_t), sizeof(uint32_t) * 2);

    spdlog::info("ReadData    {}",ReadData);
    spdlog::info("TEXDATA[0]  {}",TEXDATA[2]);

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
            ImGui::Image((ImTextureID)(intptr_t)TextureHandle,ImVec2(64,64));
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
