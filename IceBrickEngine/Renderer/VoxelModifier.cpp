#include "VoxelModifier.hpp"
#include "RHI/rhi.hpp"
#include "RHI/rhi_types.hpp"
#include "glm/vector_relational.hpp"

#include "Core/IOHelper.hpp"

namespace IceBrick::Renderer
{


    VoxelModifier::VoxelModifier()
    {
        auto* device = RHI::GetDevice();
        std::string EditSource = ReadShaderFile("../../../IceBrickEngine/Renderer/VoxelEdit.comp");
        m_EditPipeline = device->CreateComputePipeline(EditSource);

        std::string MipSource = ReadShaderFile("../../../IceBrickEngine/Renderer/MipUpdate.comp");
        m_MipPipeline = device->CreateComputePipeline(MipSource);

        RHI::BufferDescription paramsBufferDesc;
        paramsBufferDesc.memoryAccess = RHI::BufferMemoryAccess::CpuToGpu;
        paramsBufferDesc.sizeInBytes = sizeof(EditParamsUBO);
        paramsBufferDesc.usage = RHI::BufferUsage::UniformBuffer;

        m_ParamsUBO = device->CreateBuffer(paramsBufferDesc);

        m_EditResources.SetUniformBuffer(1, m_ParamsUBO);
        m_MipResources.SetUniformBuffer(2, m_ParamsUBO);
    }

    VoxelModifier::~VoxelModifier()
    {
        auto* device = RHI::GetDevice();

        device->DeleteBuffer(m_ParamsUBO);
        device->DestroyPipeline(m_EditPipeline);
        device->DestroyPipeline(m_MipPipeline);
    }

    void VoxelModifier::ApplyBrush(const VoxelVolumeRef &volume, const VoxelBrush &brush)
    {
        int r = static_cast<int>(std::ceil(brush.radius));
        glm::ivec3 rawMin = brush.center - glm::ivec3(r);
        glm::ivec3 rawMax = brush.center + glm::ivec3(r);

        if(glm::any(glm::greaterThanEqual(rawMin, volume.dimensions)) ||
           glm::any(glm::lessThan(rawMax, glm::ivec3(0))))
            return;

        glm::ivec3 dirtyMin = glm::clamp(rawMin, glm::ivec3(0), volume.dimensions - 1);
        glm::ivec3 dirtyMax = glm::clamp(rawMax, glm::ivec3(0), volume.dimensions - 1);

        EditParamsUBO params = {};
        params.dirtyMin = glm::ivec4(dirtyMin, static_cast<int>(brush.op));
        params.dirtyMax = glm::ivec4(dirtyMax, static_cast<int>(brush.shape));
        params.brushPos = glm::vec4(glm::vec3(brush.center), brush.radius);
        params.payload = glm::uvec4(brush.voxelData, 0u, 0u, 0u);

        auto* device = RHI::GetDevice();
        device->SetBufferData(m_ParamsUBO, &params, sizeof(EditParamsUBO));

        m_EditResources.SetStorageImage(0, volume.texture, RHI::ImageAccess::WriteOnly, 0);
        
        device->BindComputePipeline(m_EditPipeline);
        m_EditResources.Commit(device);
        glm::ivec3 groups = (dirtyMax - dirtyMin + 1 + 7) / 8;
        device->DispatchCompute(groups.x, groups.y, groups.z);
        device->InsertImageMemoryBarrier(RHI::MemoryBarrierBits::ShaderImageAccess);

        RebuildMips(volume, dirtyMin, dirtyMax);
    }

    void VoxelModifier::RebuildMips(const VoxelVolumeRef &volume, glm::ivec3 dirtyMin, glm::ivec3 dirtyMax)
    {
        auto* device = RHI::GetDevice();

        if(volume.mipLevels <= 1)
        {
            device->InsertImageMemoryBarrier(RHI::MemoryBarrierBits::TextureFetch);
            return;
        }

        device->BindComputePipeline(m_MipPipeline);

        glm::ivec3 mipMin = dirtyMin;
        glm::ivec3 mipMax = dirtyMax;

        for(uint32_t level = 1; level < volume.mipLevels; level++)
        {
            mipMin >>= 1;
            mipMax >>= 1;

            EditParamsUBO params;
            params.dirtyMin = glm::ivec4(mipMin, 0);
            params.dirtyMax = glm::ivec4(mipMax, 0);
            device->SetBufferData(m_ParamsUBO, &params, sizeof(EditParamsUBO));

            m_MipResources.SetStorageImage(0, volume.texture, RHI::ImageAccess::ReadOnly, level - 1);
            m_MipResources.SetStorageImage(1, volume.texture, RHI::ImageAccess::WriteOnly, level);
            m_MipResources.Commit(device);

            glm::ivec3 groups = (mipMax - mipMin + 1 + 7) / 8;
            device->DispatchCompute(groups.x, groups.y, groups.z);
            device->InsertImageMemoryBarrier(RHI::MemoryBarrierBits::ShaderImageAccess);
        }

        device->InsertImageMemoryBarrier(RHI::MemoryBarrierBits::TextureFetch);
    }

};
