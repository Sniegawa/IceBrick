#include "Core/InputManager.hpp"
#include "Core/WindowInput.hpp"
#include "Core/Window.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_pipeline_resources.hpp"
#include "RHI/rhi_types.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <string>
#include <filesystem>

using namespace IceBrick;

struct TestStruct
{
    uint32_t a = 0;
    bool b = false;
    uint64_t c = 1;
};

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

struct Voxel
{
    uint32_t mat;
};

struct SceneData
{
    glm::vec4 SunDir;
    glm::vec4 BackgroundColor;
};

int main(int argc, char** argv)
{
    auto window = Window(1280, 720, "IceBrick");
    
    auto device = RHI::InitializeDevice(window.GetNativeHandle());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window.GetNativeHandle()), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    uint32_t TEXDATA[16] = { 
    0xFF0000FF, 0xFF00FF00, 0xFF0000FF, 0xFF00FF00,
    0xFF00FF00, 0xFF0000FF, 0xFF00FF00, 0xFF0000FF,
    0xFF0000FF, 0xFF00FF00, 0xFF0000FF, 0xFF00FF00,
    0xFF00FF00, 0xFF0000FF, 0xFF00FF00, 0xFF0000FF
    };
    RHI::BufferDescription bufDescription;
    bufDescription.sizeInBytes = sizeof(TEXDATA); // - 1; // To test critical warning
    bufDescription.usage = RHI::BufferUsage::StorageBuffer;
    bufDescription.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;

    auto BufferHandle = device->CreateBuffer(bufDescription);

    device->SetBufferData(BufferHandle,TEXDATA, sizeof(TEXDATA));


    uint32_t ReadData;

    device->GetBufferData(BufferHandle,&ReadData, sizeof(uint32_t), sizeof(uint32_t) * 2);

    spdlog::info("ReadData    {}",ReadData);
    spdlog::info("TEXDATA[0]  {}",TEXDATA[2]);


    std::vector<TestStruct> testData;
    testData.push_back(TestStruct());
    testData.push_back({1, true, 0});
    testData.push_back(TestStruct());

    bufDescription.sizeInBytes = testData.size() * sizeof(TestStruct);

    auto testStructBuffer = device->CreateBuffer(bufDescription);
    device->Upload<TestStruct>(testStructBuffer,testData);

    std::array<TestStruct,3> testRead;
    device->Download<TestStruct>(testStructBuffer, testRead);

    spdlog::info("Test struct readback: {} {} {}",testRead[1].a,testRead[1].b,testRead[1].c);

    constexpr int GRID_SIZE = 32;
    std::vector<Voxel> voxelGrid(GRID_SIZE * GRID_SIZE * GRID_SIZE,{0});

    int center = GRID_SIZE / 2;
    float radius = 10.0f;

    std::mt19937 r;

    r.seed(0);

    for(int z = 0; z < GRID_SIZE; z++)
    {
        for(int y = 0; y < GRID_SIZE; y++)
        {
            for(int x = 0; x < GRID_SIZE; x++)
            {
                float dx = x - center;
                float dy = y - center;
                float dz = z - center;

                if(std::sqrt(dx*dx + dy*dy + dz*dz) <= radius)
                {
                    int flatIndex = z * (GRID_SIZE * GRID_SIZE) + y * GRID_SIZE + x;
                    voxelGrid[flatIndex].mat = r()%4 + 1;
                }
            }
        }
    }

    RHI::BufferDescription VoxelBufdesc;
    VoxelBufdesc.sizeInBytes = sizeof(Voxel) * voxelGrid.size();
    VoxelBufdesc.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;

    auto VoxelBufferHandle = device->CreateBuffer(VoxelBufdesc);

    device->Upload<Voxel>(VoxelBufferHandle,voxelGrid);

    SceneData sceneData;
    sceneData.BackgroundColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
    sceneData.SunDir = glm::vec4(0.5f, 0.8f, 0.3f, 1.0f);

    RHI::BufferDescription SceneDataDesc;
    SceneDataDesc.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;
    SceneDataDesc.usage = RHI::BufferUsage::UniformBuffer;
    SceneDataDesc.sizeInBytes = sizeof(SceneData);

    auto SceneDataBuffer = device->CreateBuffer(SceneDataDesc);

    device->Upload<SceneData>(SceneDataBuffer,sceneData);

    RHI::TextureDescription outTexture;
    outTexture.width = 1920;
    outTexture.height = 1080;
    outTexture.format = RHI::TextureFormat::RGBA8_UNORM;
    outTexture.type = RHI::TextureType::Texture2D;
    outTexture.usage = RHI::TextureUsage::Storage;

    auto outTextureHandle = device->CreateTexture(outTexture);

    RHI::PipelineResources voxelPipelineResources;
    voxelPipelineResources.SetStorageImage(0, outTextureHandle);
    voxelPipelineResources.SetStorageBuffer(1, VoxelBufferHandle);
    voxelPipelineResources.SetUniformBuffer(2, SceneDataBuffer);

    std::string VoxelRayMarchSource = ReadShaderFile("../../../IceBrickExample/src/Voxel.comp");
    auto VoxelPipeline = device->CreateComputePipeline(VoxelRayMarchSource);

    unsigned int VoxelRenderNative = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(device->GetTextureNativeHandle(outTextureHandle)));


    while(!window.ShouldClose())
    {
        glfwPollEvents();
       
        if(InputManager::IsKeyPressed(Key::Space))
            spdlog::info("SPACE");

        voxelPipelineResources.Commit(device);
        device->BindComputePipeline(VoxelPipeline);

        device->DispatchCompute(
                static_cast<uint32_t>((1920.0 + 15.0) / 16.0),
                static_cast<uint32_t>((1080.0 + 15.0) / 16.0),
                1
        );


        device->InsertImageMemoryBarrier();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

        if(ImGui::Begin("Parameters"))
        {
            ImGui::DragFloat3("Sun direction", glm::value_ptr(sceneData.SunDir),0.05f,-1.0f,1.0f);
            ImGui::ColorPicker3("Background color", glm::value_ptr(sceneData.BackgroundColor));

            if(ImGui::Button("Upload data"))
            {
                device->Upload<SceneData>(SceneDataBuffer,sceneData);
            }
        }
        ImGui::End();

        if(ImGui::Begin("Output"))
        {
            ImGui::Image((ImTextureID)(intptr_t)VoxelRenderNative,ImVec2(1920,1080));
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
