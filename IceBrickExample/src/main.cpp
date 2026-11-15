#include "Core/InputManager.hpp"
#include "Core/WindowInput.hpp"
#include "Core/Window.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_pipeline_resources.hpp"
#include "RHI/rhi_types.hpp"

#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <string>
#include <filesystem>


std::string ReadShaderFile(const std::filesystem::path& filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        
    if (!file.is_open()) {
            spdlog::error("Failed to open shader file: {}", filepath.string());
            return "";
        }

        // Grab the size, allocate the string, and rewind to the start
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::string buffer(fileSize, '\0');
        file.seekg(0);
        
        // Read the entire file in one blocking system call
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
}

int main(int argc, char** argv)
{
    auto window = IceBrick::Window(1280, 720, "IceBrick");
    
    auto device = IceBrick::RHI::InitializeDevice(window.GetNativeHandle());

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
    };
    auto texHandle = device->CreateTexture(texDescription);
    
    device->SetTextureData(texHandle, TEXDATA, sizeof(TEXDATA));
    
    unsigned int TextureHandle = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(device->GetTextureNativeHandle(texHandle)));

    IceBrick::RHI::BufferDescription bufDescription;
    bufDescription.sizeInBytes = sizeof(TEXDATA); // - 1; // To test critical warning
    bufDescription.usage = IceBrick::RHI::BufferUsage::StorageBuffer;
    bufDescription.memoryAccess = IceBrick::RHI::BufferMemoryAccess::CpuToGpu;

    auto BufferHandle = device->CreateBuffer(bufDescription);

    device->SetBufferData(BufferHandle,TEXDATA, sizeof(TEXDATA));


    uint32_t ReadData;

    device->GetBufferData(BufferHandle,&ReadData, sizeof(uint32_t), sizeof(uint32_t) * 2);

    spdlog::info("ReadData    {}",ReadData);
    spdlog::info("TEXDATA[0]  {}",TEXDATA[2]);


    // Pipeline tests

    std::string compSource = ReadShaderFile("../../../IceBrickExample/src/uv.comp");
    auto uvPipeline = device->CreateComputePipeline(compSource);

    IceBrick::RHI::TextureDescription uvDescription;
    uvDescription.width = 1920;
    uvDescription.height = 1080;
    uvDescription.format = IceBrick::RHI::TextureFormat::RGBA8_UNORM;
    uvDescription.type = IceBrick::RHI::TextureType::Texture2D;
    uvDescription.usage = IceBrick::RHI::TextureUsage::Storage;

    auto uvTexture = device->CreateTexture(uvDescription);
    

    IceBrick::RHI::PipelineResources resources;
    resources.SetStorageImage(0, uvTexture, IceBrick::RHI::ImageAccess::WriteOnly);

    while(!window.ShouldClose())
    {
        glfwPollEvents();
       
        if(IceBrick::InputManager::IsKeyPressed(IceBrick::Key::Space))
            spdlog::info("SPACE");

        resources.Commit(device);
        device->BindComputePipeline(uvPipeline);

        device->DispatchCompute(static_cast<uint32_t>((1920.0 + 15.0) / 16.0), static_cast<uint32_t>((1080.0 + 15.0) / 16.0),1);
        
        device->InsertImageMemoryBarrier();


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
        device->ClearScreen(0.1f, 0.11f, 0.12f);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.SwapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
