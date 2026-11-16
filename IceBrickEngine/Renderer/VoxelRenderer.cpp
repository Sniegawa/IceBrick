#include "VoxelRenderer.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_handles.hpp"
#include "Renderer/VoxelFormat.hpp"

#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>

namespace IceBrick::Renderer 
{
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

    VoxelRenderer::VoxelRenderer(RHI::BufferHandle sceneDataBuffer)
    {
        auto* device = RHI::GetDevice();

        RHI::TextureDescription outTexture;
        outTexture.width = 1920;
        outTexture.height = 1080;
        outTexture.format = RHI::TextureFormat::RGBA8_UNORM;
        outTexture.type = RHI::TextureType::Texture2D;
        outTexture.usage = RHI::TextureUsage::Storage;
        outTexture.filter = RHI::TextureFilter::Linear;
        outTexture.generateMips = false;

        m_OutputTexture = device->CreateTexture(outTexture);


        RHI::BufferDescription VoxelBufdesc;
        VoxelBufdesc.sizeInBytes = sizeof(Voxel) * GRID_SIZE * GRID_SIZE * GRID_SIZE;
        VoxelBufdesc.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;

        m_VoxelBuffer = device->CreateBuffer(VoxelBufdesc);

        std::string voxelSource = ReadShaderFile("../../../IceBrickExample/src/Voxel.comp");
        m_VoxelPipeline = device->CreateComputePipeline(voxelSource);

        m_VoxelPipelineResources.SetStorageImage(0, m_OutputTexture);
        m_VoxelPipelineResources.SetStorageBuffer(1, m_VoxelBuffer);
        m_VoxelPipelineResources.SetUniformBuffer(2, sceneDataBuffer);
    }
    VoxelRenderer::~VoxelRenderer()
    {
        auto* device = RHI::GetDevice();

        device->DestroyTexture(m_OutputTexture);
        device->DeleteBuffer(m_VoxelBuffer);
        device->DestroyPipeline(m_VoxelPipeline);
    }

    void VoxelRenderer::Draw()
    {
        auto* device = RHI::GetDevice();

        m_VoxelPipelineResources.Commit(device);

        device->BindComputePipeline(m_VoxelPipeline);

        // TODO : CHANGE SO THAT IT USES THE OUT TEXTURE RESOLUTION DYNAMICALLY
        device->DispatchCompute(
                static_cast<uint32_t>((1920.0 + 15.0) / 16.0),
                static_cast<uint32_t>((1080.0 + 15.0) / 16.0),
                1
        );

        device->InsertImageMemoryBarrier();
    }

    void VoxelRenderer::UploadVoxelData(std::span<Voxel> data)
    {
        RHI::GetDevice()->Upload<Voxel>(m_VoxelBuffer,data);
    }
}
