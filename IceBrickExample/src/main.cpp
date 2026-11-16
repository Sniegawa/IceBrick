#include "Core/InputManager.hpp"
#include "Core/WindowInput.hpp"
#include "Core/Window.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_types.hpp"
#include "Renderer/VoxelFormat.hpp"
#include "Renderer/VoxelRenderer.hpp"

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


#include "Camera.hpp"
#include "Voxelizer.hpp"

using namespace IceBrick;

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

Camera camera;
bool isCameraActive = false;

struct SceneData
{
    glm::vec4 SunDir;
    glm::vec4 BackgroundColor;

    glm::vec4 camPos;
    glm::vec4 camFront;
    glm::vec4 camUp;
    glm::vec4 camRight;
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

    SceneData sceneData;
    sceneData.BackgroundColor = glm::vec4(0.5f, 0.7f, 1.0f, 1.0f);
    sceneData.SunDir = glm::vec4(0.5f, 0.8f, 0.3f, 1.0f);

    RHI::BufferDescription SceneDataDesc;
    SceneDataDesc.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;
    SceneDataDesc.usage = RHI::BufferUsage::UniformBuffer;
    SceneDataDesc.sizeInBytes = sizeof(SceneData);

    auto SceneDataBuffer = device->CreateBuffer(SceneDataDesc);

    device->Upload<SceneData>(SceneDataBuffer,sceneData);


    Renderer::VoxelRenderer renderer(SceneDataBuffer);


    constexpr int GRID_SIZE = Renderer::GRID_SIZE;
    std::vector<Renderer::Voxel> voxelGrid(GRID_SIZE * GRID_SIZE * GRID_SIZE,{0});

    float center = GRID_SIZE / 2.0f;
    float majorRadius = 40.0f; 
    float minorRadius = 10.0f;

    std::mt19937 rng(6767);
    std::uniform_int_distribution<int> colorDist(0,255);

    for (int z = 0; z < GRID_SIZE; ++z) {
        for (int y = 0; y < GRID_SIZE; ++y) {
            for (int x = 0; x < GRID_SIZE; ++x) {
            
                float dx = x - center;
                float dy = y - center; 
                float dz = z - center;
            
                float distXZ = std::sqrt(dx*dx + dz*dz);
            
                float distToTube = distXZ - majorRadius;
            
                if ((distToTube * distToTube + dy * dy) <= (minorRadius * minorRadius)) {
                
                    size_t flatIndex = z * (GRID_SIZE * GRID_SIZE) + y * GRID_SIZE + x;
                
                    uint8_t r = static_cast<uint8_t>(colorDist(rng));
                    uint8_t g = static_cast<uint8_t>(colorDist(rng));
                    uint8_t b = static_cast<uint8_t>(colorDist(rng));

                    voxelGrid[flatIndex].data = Renderer::Voxel::Pack(r, g, b, 1);
               }

                if(y == 0)
                { 
                    size_t flatIndex = z * (GRID_SIZE * GRID_SIZE) + y * GRID_SIZE + x;

                    uint8_t r = static_cast<uint8_t>(colorDist(rng));
                    uint8_t g = static_cast<uint8_t>(colorDist(rng));
                    uint8_t b = static_cast<uint8_t>(colorDist(rng));

                    voxelGrid[flatIndex].data = Renderer::Voxel::Pack(r, g, b, 1);               
                }
            }
        }
    }
    
    renderer.UploadVoxelData(voxelGrid);


    unsigned int VoxelRenderNative = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(device->GetTextureNativeHandle(renderer.GetOutTextureHandle())));

    MeshData meshData = Load("../../../IceBrickExample/src/Dragon.gltf");
    VoxelMesh voxelizedMesh = Voxelize(meshData);

    renderer.UploadVoxelData(voxelizedMesh.VoxelMesh);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    while(!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
       
        renderer.Draw();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

        if(ImGui::Begin("Parameters"))
        {
            bool flag = false;
            if(ImGui::DragFloat3("Sun direction", glm::value_ptr(sceneData.SunDir),0.05f,-1.0f,1.0f)) flag = true;
            if(ImGui::ColorPicker3("Background color", glm::value_ptr(sceneData.BackgroundColor))) flag = true;
            if(flag)
                device->Upload<SceneData>(SceneDataBuffer,sceneData);

        }
        ImGui::End();

        if(ImGui::Begin("Viewport"))
        {
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();


            ImGui::Image((ImTextureID)(intptr_t)VoxelRenderNative, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

            if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                if(!isCameraActive)
                {
                    isCameraActive = true;
                    window.DisableCursor();
                }
            }
        }
        ImGui::End();

        if(ImGui::Begin("Stats"))
        {
            ImGui::Text("ms: %f",1000.0f * deltaTime);
            ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
        }
        ImGui::End();

        if(isCameraActive && InputManager::IsKeyPressed(Key::Escape))
        {
            isCameraActive = false;
            window.EnableCursor();
        }

        if(isCameraActive)
        {
            camera.Update(deltaTime, window.GetNativeHandle());
            sceneData.camPos = glm::vec4(camera.GetPosition(),1.0f);
            sceneData.camFront = glm::vec4(camera.GetFront(),1.0f);
            sceneData.camRight = glm::vec4(camera.GetRight(), 1.0f);
            sceneData.camUp = glm::vec4(camera.GetUp(), 1.0f);
            device->Upload<SceneData>(SceneDataBuffer,sceneData);
        }


        ImGui::Render();
        device->ClearScreen(0.1f, 0.11f, 0.12f);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.SwapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    
}
