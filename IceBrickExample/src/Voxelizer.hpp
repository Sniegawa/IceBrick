#pragma once

#include "Renderer/VoxelFormat.hpp"
#include "Renderer/VoxelRenderer.hpp"
#include "glm/glm.hpp"
#include <filesystem>
#include <limits>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
};

struct TextureData 
{
    int width; 
    int height; 
    std::vector<uint8_t> pixels; 
};

struct SubMesh
{
    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
    int32_t textureIndex = -1;
    glm::vec3 baseColorFactor = glm::vec3(1.0f);
};


struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh> subMeshes;
    std::vector<TextureData> textures;

    glm::vec3 localMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 localMax = glm::vec3(std::numeric_limits<float>::lowest());
};

struct VoxelMesh
{
    std::vector<IceBrick::Renderer::Voxel> VoxelMesh;
    std::vector<std::vector<IceBrick::Renderer::Voxel>> Mips;
};

MeshData Load(const std::filesystem::path& filepath);


VoxelMesh Voxelize(const MeshData& data);

