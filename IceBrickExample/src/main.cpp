#include "Core/InputManager.hpp"
#include "Core/WindowInput.hpp"
#include "Core/Window.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_types.hpp"
#include "Renderer/VoxelFormat.hpp"
#include "Renderer/VoxelModifier.hpp"
#include "Renderer/VoxelRenderer.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <filesystem>


#include "Camera.hpp"
#include "Voxelizer.hpp"

using namespace IceBrick;

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
    glm::ivec4 debugInfo = glm::ivec4(0,100,0,0);
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
    Renderer::VoxelModifier modifier;
    Renderer::VoxelBrush brush;
    brush.radius    = 10.0f;
    brush.voxelData = Renderer::Voxel::Pack(255, 110, 30, true); 
    brush.shape     = Renderer::BrushShape::Sphere;
    brush.op        = Renderer::BrushOp::Place;
    glm::vec3 RawBrushColor = glm::vec3(0.0f);




    Renderer::VoxelVolumeRef volumeRef = renderer.getVolumeRef();

    unsigned int VoxelRenderNative = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(device->GetTextureNativeHandle(renderer.GetOutTextureHandle())));

    {
        MeshData meshData = Load("../../../IceBrickExample/src/MaterialsVariantsShoe.glb");
        VoxelMesh voxelizedMesh = Voxelize(meshData);

        renderer.UploadVoxelData(voxelizedMesh.VoxelMesh, 0);
    }
    modifier.RebuildMips(volumeRef, glm::ivec3(0), volumeRef.dimensions - 1);

    Renderer::RaycastHit LastHit;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    
    float brushCooldown = 0.0f;

    while(!window.ShouldClose())
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        LastHit = renderer.getHitData();
        glfwPollEvents();
        
        
        brushCooldown += deltaTime;
        if(isCameraActive && brushCooldown >= 0.06f)
        {
            bool carve = InputManager::IsKeyPressed(Key::E);
            bool place = InputManager::IsKeyPressed(Key::Q);

            if ((carve || place) && LastHit.valid)
            {
                brush.op = carve ? Renderer::BrushOp::Carve : Renderer::BrushOp::Place;
                brush.center = carve ? LastHit.cell : (LastHit.cell + LastHit.normal * static_cast<int>(brush.radius));

                auto to8Bit = [](float c) -> uint8_t {
                    return static_cast<uint8_t>(glm::clamp(c, 0.0f, 1.0f) * 255.0f + 0.5f);
                };
                brush.voxelData = Renderer::Voxel::Pack(to8Bit(RawBrushColor.r),to8Bit(RawBrushColor.g),to8Bit(RawBrushColor.b),true);

                modifier.ApplyBrush(volumeRef, brush);
                brushCooldown = 0.0f;
            }
        }

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
            if(ImGui::DragInt("Debug mode", &sceneData.debugInfo.x,1,0,1)) flag = true;
            if(ImGui::DragInt("Debug threshold", &sceneData.debugInfo.y,1,20,1000)) flag = true;


            if(flag)
                device->Upload<SceneData>(SceneDataBuffer,sceneData);


            ImGui::Spacing();
            ImGui::Text("Brush");
            ImGui::DragFloat("Brush radius", &brush.radius, 0.5f, 1.0f, 40.0f);
            int shape = static_cast<int>(brush.shape);
            ImGui::DragInt("Brush type", &shape, 1.0f, 0, 1);
            brush.shape = static_cast<Renderer::BrushShape>(shape);

            ImGui::ColorEdit3("Brush color", glm::value_ptr(RawBrushColor));
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
