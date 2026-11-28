#include "VoxelRenderer.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_types.hpp"
#include "Renderer/VoxelFormat.hpp"

#include "Core/IOHelper.hpp"

namespace IceBrick::Renderer 
{
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

        RHI::TextureDescription voxelTexDescription;
        voxelTexDescription.width = GRID_SIZE;
        voxelTexDescription.height = GRID_SIZE;
        voxelTexDescription.depth = GRID_SIZE;

        voxelTexDescription.format = RHI::TextureFormat::R16_UINT;
        voxelTexDescription.type = RHI::TextureType::Texture3D;
        voxelTexDescription.usage = RHI::TextureUsage::Sampled | RHI::TextureUsage::Bindless;
        voxelTexDescription.filter = RHI::TextureFilter::Nearest;
        voxelTexDescription.generateMips = true;

        m_Volume.texture = device->CreateTexture(voxelTexDescription);

        RHI::BufferDescription volumeHandleBufferDesc;
        volumeHandleBufferDesc.usage = RHI::BufferUsage::UniformBuffer;
        volumeHandleBufferDesc.sizeInBytes = sizeof(VolumeHandleUBO);
        volumeHandleBufferDesc.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;
        
        m_VolumeHandleBuffer = device->CreateBuffer(volumeHandleBufferDesc);

        VolumeHandleUBO data{ device->GetTextureBindlessHandle(m_Volume.texture) };
        device->Upload(m_VolumeHandleBuffer, data);

        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(GRID_SIZE))) + 1;
        m_Volume.mipLevels = mipLevels;
        m_Volume.dimensions = glm::ivec3(GRID_SIZE);

        RHI::BufferDescription raycastHitDesc;
        raycastHitDesc.sizeInBytes = sizeof(RaycastHit);
        raycastHitDesc.usage = RHI::BufferUsage::StorageBuffer;
        raycastHitDesc.memoryAccess = RHI::BufferMemoryAccess::GpuToCpu;
        m_RaycastHitBuffer = device->CreateBuffer(raycastHitDesc);

        std::string voxelSource = ReadShaderFile("../../../IceBrickEngine/Renderer/Voxel.comp");
        m_VoxelPipeline = device->CreateComputePipeline(voxelSource);

        m_VoxelPipelineResources.SetStorageImage(0, m_OutputTexture);
        m_VoxelPipelineResources.SetUniformBuffer(1, m_VolumeHandleBuffer);
        m_VoxelPipelineResources.SetUniformBuffer(2, sceneDataBuffer);
        m_VoxelPipelineResources.SetStorageBuffer(3, m_RaycastHitBuffer);
    }
    VoxelRenderer::~VoxelRenderer()
    {
        auto* device = RHI::GetDevice();

        device->DestroyTexture(m_OutputTexture);
        device->DestroyTexture(m_Volume.texture);
        device->DeleteBuffer(m_RaycastHitBuffer);
        device->DestroyPipeline(m_VoxelPipeline);
    }

    void VoxelRenderer::Draw()
    {
        auto* device = RHI::GetDevice();

        m_VoxelPipelineResources.Commit(device);

        device->BindComputePipeline(m_VoxelPipeline);

        // TODO : CHANGE SO THAT IT USES THE OUT TEXTURE RESOLUTION DYNAMICALLY
        device->DispatchCompute(
                static_cast<uint32_t>((1920.0 + 7.0) / 8.0),
                static_cast<uint32_t>((1080.0 + 7.0) / 8.0),
                1
        );

        device->InsertImageMemoryBarrier(RHI::MemoryBarrierBits::ShaderImageAccess);
    }

    void VoxelRenderer::UploadVoxelData(std::span<Voxel> data, int mip)
    {
        RHI::GetDevice()->SetTextureData(m_Volume.texture, data.data(), data.size_bytes(), mip);
    }

    RaycastHit VoxelRenderer::getHitData()
    {
        RaycastHit hit;
        auto* device = RHI::GetDevice();
        device->GetBufferData(m_RaycastHitBuffer, &hit, sizeof(RaycastHit));
        return hit;
    }
}
