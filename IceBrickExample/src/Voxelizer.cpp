#include "Voxelizer.hpp"
#include "Renderer/VoxelFormat.hpp"
#include "Renderer/VoxelRenderer.hpp"
#include "fastgltf/error.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/parser.hpp"
#include "glm/common.hpp"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

MeshData Load(const std::filesystem::path& filepath)
{
    MeshData result;

    fastgltf::Parser parser;

    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;

    auto data = fastgltf::GltfDataBuffer::FromPath(filepath);
    if(data.error() != fastgltf::Error::None)
    {
        spdlog::error("Failed to load glTF file: {}", filepath.string());
        return result;
    }

    auto expectedAsset = parser.loadGltf(data.get(), filepath.parent_path(), gltfOptions);
    if(expectedAsset.error() != fastgltf::Error::None)
    {
        spdlog::error("fastgltf parsing error: {}", fastgltf::getErrorMessage(expectedAsset.error()));   
        return result;
    }

    fastgltf::Asset asset = std::move(expectedAsset.get());

    for(const auto& mesh : asset.meshes)
    {
        for(const auto& primitive : mesh.primitives)
        {
            if(primitive.type != fastgltf::PrimitiveType::Triangles) continue;

            uint32_t initialVertexCount = static_cast<uint32_t>(result.vertices.size());

            auto posIt = primitive.findAttribute("POSITION");
            if(posIt != primitive.attributes.end())
            {
                auto& accessor = asset.accessors[posIt->accessorIndex];

                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, accessor,
                        [&](glm::vec3 pos, size_t idx)
                        {
                            result.vertices.push_back({ pos, glm::vec3(0.7f,0.7f,0.7f)});
                            if(pos.x < result.localMin.x) result.localMin.x = pos.x;
                            if(pos.y < result.localMin.y) result.localMin.y = pos.y;
                            if(pos.z < result.localMin.z) result.localMin.z = pos.z;
                            
                            if(pos.x > result.localMax.x) result.localMax.x = pos.x;
                            if(pos.y > result.localMax.y) result.localMax.y = pos.y;
                            if(pos.z > result.localMax.z) result.localMax.z = pos.z;
                        }
                );
            }

            auto colorIt = primitive.findAttribute("COLOR_0");
            if(colorIt != primitive.attributes.end())
            {
                auto& accessor = asset.accessors[colorIt->accessorIndex];

                fastgltf::iterateAccessorWithIndex<glm::vec4>(asset, accessor, 
                        [&](glm::vec3 color, size_t index)
                        {
                            result.vertices[initialVertexCount + index].color = glm::vec3(color);    
                            //spdlog::info("{} {} {}",color.r, color.g, color.b);
                        }
                );
            }

            if(primitive.indicesAccessor.has_value())
            {
                auto& accessor = asset.accessors[primitive.indicesAccessor.value()];

                fastgltf::iterateAccessorWithIndex<uint32_t>(asset, accessor, 
                        [&](uint32_t index, size_t) // Sorry...
                        {
                            result.indices.push_back(index + initialVertexCount);
                        }
                );
            } 
            else 
            {
                uint32_t vertexCount = static_cast<uint32_t>(result.vertices.size()) - initialVertexCount;
                for(uint32_t i = 0; i < vertexCount; i++)
                    result.indices.push_back(initialVertexCount + 1);
            }

        }
    }

    spdlog::info("Successfully loaded {}, Vertices: {}, Indices: {}",
            filepath.string(), result.vertices.size(), result.indices.size());

    return result;
}


VoxelMesh Voxelize(MeshData data)
{
    VoxelMesh vMesh;

    constexpr size_t GRID_SIZE = IceBrick::Renderer::GRID_SIZE;

    vMesh.VoxelMesh.resize(GRID_SIZE * GRID_SIZE * GRID_SIZE, IceBrick::Renderer::Voxel{0});

    glm::vec3 size = data.localMax - data.localMin;
    glm::vec3 center = data.localMin + (size * 0.5f);
    
    float TargetSize = static_cast<float>(GRID_SIZE - 1);

    float maxDimension = std::max(std::max(size.x,size.y),size.z);

    float scaleFactor = TargetSize / maxDimension; // TODO Check if there is no divide by 0

    for(size_t i = 0; i < data.indices.size(); i += 3)
    {
        auto V0 = data.vertices[data.indices[i + 0]];
        auto V1 = data.vertices[data.indices[i + 1]];
        auto V2 = data.vertices[data.indices[i + 2]];

        auto P0 = V0.position - center;
        auto P1 = V1.position - center;
        auto P2 = V2.position - center;

        P0 *= scaleFactor;
        P1 *= scaleFactor;
        P2 *= scaleFactor;

        P0 += glm::vec3(GRID_SIZE / 2.0f);
        P1 += glm::vec3(GRID_SIZE / 2.0f);
        P2 += glm::vec3(GRID_SIZE / 2.0f);

        glm::vec3 E0 = P1 - P0;
        glm::vec3 E1 = P2 - P1;
        glm::vec3 E2 = P0 - P2;

        glm::vec3 N = glm::cross(E0,E1);

        glm::ivec3 triMin = glm::floor(glm::min(glm::min(P0,P1),P2)); 
        glm::ivec3 triMax = glm::ceil(glm::max(glm::max(P0,P1),P2));

        triMin = glm::clamp(triMin, glm::ivec3(0), glm::ivec3(GRID_SIZE - 1));
        triMax = glm::clamp(triMax, glm::ivec3(0), glm::ivec3(GRID_SIZE - 1));


        for(int z = triMin.z; z <= triMax.z; z++)
        {
            for(int y = triMin.y; y <= triMax.y; y++)
            {
                for(int x = triMin.x; x <= triMax.x; x++)
                {
                    glm::vec3 voxelCenter = glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f);


                    float centerDistance = glm::dot(N,(voxelCenter - P0));

                    float ProjectedRadius = 0.5f * (std::abs(N.x) + std::abs(N.y) + std::abs(N.z));
                    
                    // 4th axis
                    if(std::abs(centerDistance) > ProjectedRadius) continue;


                    glm::vec3 v0 = P0 - voxelCenter;
                    glm::vec3 v1 = P1 - voxelCenter;
                    glm::vec3 v2 = P2 - voxelCenter;

                    float p0, p1, p2, r, minP, maxP;

                    //5. X-Axis x E0
                    p0 = v0.z * E0.y - v0.y * E0.z;
                    p2 = v2.z * E0.y - v2.y * E0.z;
                    minP = std::min(p0, p2);
                    maxP = std::max(p0, p2);
                    r = 0.5f * (std::abs(E0.z) + std::abs(E0.y));
                    if(minP > r || maxP < - r) continue;
                    
                    // 6. X-Axis x E1
                    p0 = v0.z * E1.y - v0.y * E1.z;
                    p1 = v1.z * E1.y - v1.y * E1.z;
                    minP = std::min(p0, p1); 
                    maxP = std::max(p0, p1);
                    r = 0.5f * (std::abs(E1.z) + std::abs(E1.y));
                    if (minP > r || maxP < -r) continue;

                    // 7. X-Axis x E2
                    p0 = v0.z * E2.y - v0.y * E2.z;
                    p1 = v1.z * E2.y - v1.y * E2.z;
                    minP = std::min(p0, p1); 
                    maxP = std::max(p0, p1);
                    r = 0.5f * (std::abs(E2.z) + std::abs(E2.y));
                    if (minP > r || maxP < -r) continue;

                    // 8. Y-Axis x E0
                    p0 = -v0.z * E0.x + v0.x * E0.z;
                    p2 = -v2.z * E0.x + v2.x * E0.z;
                    minP = std::min(p0, p2); 
                    maxP = std::max(p0, p2);
                    r = 0.5f * (std::abs(E0.z) + std::abs(E0.x));
                    if (minP > r || maxP < -r) continue;

                    // 9. Y-Axis x E1
                    p0 = -v0.z * E1.x + v0.x * E1.z;
                    p1 = -v1.z * E1.x + v1.x * E1.z;
                    minP = std::min(p0, p1); 
                    maxP = std::max(p0, p1);
                    r = 0.5f * (std::abs(E1.z) + std::abs(E1.x));
                    if (minP > r || maxP < -r) continue;

                    // 10. Y-Axis x E2
                    p0 = -v0.z * E2.x + v0.x * E2.z;
                    p1 = -v1.z * E2.x + v1.x * E2.z;
                    minP = std::min(p0, p1); 
                    maxP = std::max(p0, p1);
                    r = 0.5f * (std::abs(E2.z) + std::abs(E2.x));
                    if (minP > r || maxP < -r) continue;

                    // 11. Z-Axis x E0
                    p0 = v0.y * E0.x - v0.x * E0.y;
                    p2 = v2.y * E0.x - v2.x * E0.y;
                    minP = std::min(p0, p2); 
                    maxP = std::max(p0, p2);
                    r = 0.5f * (std::abs(E0.y) + std::abs(E0.x));
                    if (minP > r || maxP < -r) continue;

                    // 12. Z-Axis x E1
                    p0 = v0.y * E1.x - v0.x * E1.y;
                    p1 = v1.y * E1.x - v1.x * E1.y;
                    minP = std::min(p0, p1); 
                    maxP = std::max(p0, p1);
                    r = 0.5f * (std::abs(E1.y) + std::abs(E1.x));
                    if (minP > r || maxP < -r) continue;

                    // 13. Z-Axis x E2
                    p0 = v0.y * E2.x - v0.x * E2.y;
                    p1 = v1.y * E2.x - v1.x * E2.y;
                    minP = std::min(p0, p1); 
                    maxP = std::max(p0, p1);
                    r = 0.5f * (std::abs(E2.y) + std::abs(E2.x));
                    if (minP > r || maxP < -r) continue;

                    // SUCCESS
                    
                    glm::vec3 avgColor = (V0.color + V1.color + V2.color) / 3.0f;

                    uint8_t rCol = static_cast<uint8_t>(glm::clamp(avgColor.r * 255.0f, 0.0f, 255.0f));
                    uint8_t gCol = static_cast<uint8_t>(glm::clamp(avgColor.g * 255.0f, 0.0f, 255.0f));
                    uint8_t bCol = static_cast<uint8_t>(glm::clamp(avgColor.b * 255.0f, 0.0f, 255.0f));

                    uint32_t packedData = IceBrick::Renderer::Voxel::Pack(rCol, gCol, bCol, 1);

                    size_t flatIndex = static_cast<size_t>(z) * (GRID_SIZE * GRID_SIZE) + static_cast<size_t>(y) * (GRID_SIZE) + static_cast<size_t>(x);

                    vMesh.VoxelMesh[flatIndex].data = packedData;
                }
            }
        }


    }



    return vMesh;
}
