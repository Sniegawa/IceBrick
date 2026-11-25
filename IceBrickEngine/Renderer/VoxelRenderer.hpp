#pragma once

#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_pipeline_resources.hpp"
#include "Renderer/VoxelFormat.hpp"

#include <glm/glm.hpp>
#include <span>

namespace IceBrick::Renderer
{
    // TODO : MOVE IT SOMEWHERE ELSE
    struct SceneData
    {
        glm::vec4 SunDir;
        glm::vec4 BackgroundColor;

        glm::vec4 camPos;
        glm::vec4 camFront;
        glm::vec4 camUp;
        glm::vec4 camRight;
    };

    constexpr size_t GRID_SIZE = 512;

    class VoxelRenderer
    {
    public:
        VoxelRenderer(RHI::BufferHandle sceneDataBuffer);
        ~VoxelRenderer();

        void Draw();

        void UploadVoxelData(std::span<Voxel> data, int mip);

        RHI::TextureHandle GetOutTextureHandle() { return m_OutputTexture; }

    private:
        RHI::TextureHandle m_VoxelTexture;
        RHI::TextureHandle m_OutputTexture;

        RHI::PipelineResources m_VoxelPipelineResources;
        RHI::PipelineHandle m_VoxelPipeline;
    };
};
