#pragma once

#include "Renderer/VoxelFormat.hpp"
#include "Renderer/VoxelRenderer.hpp"
#include "glm/glm.hpp"
#include <filesystem>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 localMin;
    glm::vec3 localMax;
};

struct VoxelMesh
{
    std::vector<IceBrick::Renderer::Voxel> VoxelMesh;
};

MeshData Load(const std::filesystem::path& filepath);


VoxelMesh Voxelize(MeshData data);
