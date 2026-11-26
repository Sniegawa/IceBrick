#pragma once

#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_pipeline_resources.hpp"
#include "RHI/rhi_types.hpp"
#include "VoxelFormat.hpp"
#include <cstdint>
#include <glm/glm.hpp>

namespace IceBrick::Renderer
{
    // Non owning handle
    struct VoxelVolumeRef
    {
        RHI::TextureHandle texture{};
        glm::ivec3 dimensions{0};
        uint32_t mipLevels = 1;
    };

    enum class BrushShape : uint32_t
    {
        Sphere = 0,
        Box = 1
    };

    enum class BrushOp : uint32_t
    {
        Carve = 0,
        Place = 1
    };

    struct VoxelBrush
    {
        glm::ivec3 center{0};
        float radius = 4.0f;
        uint16_t voxelData = 0; // Packed color data + opaque flag
        BrushShape shape = BrushShape::Sphere;
        BrushOp op = BrushOp::Carve;
    };

    class VoxelModifier
    {
    public:
        VoxelModifier();
        ~VoxelModifier();

        void ApplyBrush(const VoxelVolumeRef& volume, const VoxelBrush& brush);

        void RebuildMips(const VoxelVolumeRef& volume, glm::ivec3 dirtyMin, glm::ivec3 dirtyMax);

    private:
        struct alignas(16) EditParamsUBO
        {
            glm::ivec4 dirtyMin;
            glm::ivec4 dirtyMax;
            glm::vec4 brushPos;
            glm::uvec4 payload;
        };

        RHI::PipelineResources m_EditResources;
        RHI::PipelineHandle m_EditPipeline;

        RHI::PipelineResources m_MipResources;
        RHI::PipelineHandle m_MipPipeline;

        RHI::BufferHandle m_ParamsUBO{};
    };


};
