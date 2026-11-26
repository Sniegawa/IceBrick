#include "Voxelizer.hpp"
#include "Renderer/VoxelFormat.hpp"
#include "Renderer/VoxelRenderer.hpp"
#include "fastgltf/error.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/parser.hpp"
#include "glm/common.hpp"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/vector_float2.hpp"
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

#define STB_IMAGE_IMPLEMENTATION
#include <Vendor/stb/stb_image.h>



MeshData Load(const std::filesystem::path& filepath)
{
    MeshData result;

    fastgltf::Parser parser;

    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages;

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
                            result.vertices.push_back({ pos, glm::vec3(1.0f,1.0f,1.0f), glm::vec3(0.0f)});
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
                        }
                );
            }

            auto uvIt = primitive.findAttribute("TEXCOORD_0");
            if(uvIt != primitive.attributes.end())
            {
                auto& accessor = asset.accessors[uvIt->accessorIndex];

                fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, accessor, 
                        [&](glm::vec2 uv, size_t index)
                        {
                            result.vertices[initialVertexCount + index].uv = uv;
                        });
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


            // Handle textures
            stbi_set_flip_vertically_on_load(false);

            glm::vec3 baseColorFactor(1.0f);
            if(primitive.materialIndex.has_value())
            {
                const auto& mat = asset.materials[*primitive.materialIndex];
                baseColorFactor = {
                    mat.pbrData.baseColorFactor.x(),
                    mat.pbrData.baseColorFactor.y(),
                    mat.pbrData.baseColorFactor.z()
                };

                if(mat.pbrData.baseColorTexture.has_value())
                {
                    const auto& tex = asset.textures[mat.pbrData.baseColorTexture->textureIndex];
                    if(tex.imageIndex.has_value())
                    {
                        const auto& img = asset.images[*tex.imageIndex];
                        auto decodeFromMemory = [&](const std::byte* bytes, size_t len)
                        {
                            int w = 0;
                            int h = 0;
                            int channels = 0;
                            stbi_uc* raw = stbi_load_from_memory(
                                    reinterpret_cast<const stbi_uc*>(bytes),
                                    static_cast<int>(len),
                                    &w,
                                    &h,
                                    &channels,
                                    4
                            );

                            if(raw)
                            {
                                result.texture.width = w;
                                result.texture.height = h;
                                result.texture.pixels.assign(raw, raw + (w * h * 4));
                                stbi_image_free(raw);
                            }
                            else 
                            {
                                spdlog::error("stbi_load_from_memory failed: {}", stbi_failure_reason());
                            }
                        };

                        std::visit(fastgltf::visitor
                        {
                            [](auto&){},
                            [&](const fastgltf::sources::Array& arr)
                            {
                                decodeFromMemory(arr.bytes.data(), arr.bytes.size());
                            },
                            [&](const fastgltf::sources::Vector& vec)
                            {
                                decodeFromMemory(vec.bytes.data(), vec.bytes.size());
                            },
                            [&](const fastgltf::sources::BufferView& view)
                            {
                                const auto& bv = asset.bufferViews[view.bufferViewIndex];
                                const auto& buf = asset.buffers[bv.bufferIndex];
                                std::visit(fastgltf::visitor 
                                {
                                    [](auto&){},
                                    [&](const fastgltf::sources::Array& arr)
                                    {
                                        decodeFromMemory(arr.bytes.data() + bv.byteOffset, arr.bytes.size());
                                    },
                                    [&](const fastgltf::sources::Vector& vec)
                                    {
                                        decodeFromMemory(vec.bytes.data() + bv.byteOffset, vec.bytes.size());
                                    },
                                },buf.data);
                            }
                        },img.data);
                    }
                }
            }

        }
    }

    spdlog::info("Successfully loaded {}, Vertices: {}, Indices: {}",
            filepath.string(), result.vertices.size(), result.indices.size());

    return result;
}


void GenerateSupercoverMips(VoxelMesh& mesh, uint32_t gridSize, uint32_t totalMipLevels)
{
    mesh.Mips.clear();
    if(totalMipLevels <= 1 || gridSize <= 1)
        return;

    mesh.Mips.reserve(totalMipLevels - 1);

    uint32_t prevDim = gridSize;
    for(uint32_t level = 1; level < totalMipLevels; level++)
    {
        uint32_t currDim = std::max(1u, prevDim >> 1);
        std::vector<IceBrick::Renderer::Voxel> currMip(static_cast<size_t>(currDim) * currDim * currDim);

        auto readPrev = [&](uint32_t x, uint32_t y, uint32_t z) -> uint32_t // Yuck
        {
            size_t idx = static_cast<size_t>(z) * prevDim * prevDim + y * prevDim + x;
            if(level == 1)
                return mesh.VoxelMesh[idx].data;
            return mesh.Mips[level - 2][idx].data;
        };

        for(uint32_t z = 0; z < currDim; z++)
        {
            uint32_t z0 = z * 2;
            uint32_t z1 = std::min(z0 + 1, prevDim - 1);

            for(uint32_t y = 0; y < currDim; y++)
            {
                uint32_t y0 = y * 2;
                uint32_t y1 = std::min(y0 + 1, prevDim - 1);

                for(uint32_t x = 0; x < currDim; x++)
                {
                    uint32_t x0 = x * 2;
                    uint32_t x1 = std::min(x0 + 1, prevDim - 1);

                    uint32_t acc = 
                        readPrev(x0, y0, z0) | readPrev(x1, y0, z0) |
                        readPrev(x0, y1, z0) | readPrev(x1, y1, z0) |
                        readPrev(x0, y0, z1) | readPrev(x1, y0, z1) |
                        readPrev(x0, y1, z1) | readPrev(x1, y1, z1);

                    size_t dstIdx = static_cast<size_t>(z) * currDim * currDim + y * currDim + x;
                    currMip[dstIdx].data = (acc != 0u) ? 1u : 0u;
                }
            }
        }

        mesh.Mips.push_back(std::move(currMip));
        prevDim = currDim;
        if(prevDim == 1)
            break;
    }
}


glm::vec3 SampleTexture(const TextureData& tex, glm::vec2 uv)
{
    if(tex.pixels.empty() || tex.width == 0 || tex.height == 0)
        return glm::vec3(1.0f);

    float wrappedU = uv.x - std::floor(uv.x);
    float wrappedV = uv.y - std::floor(uv.y);

    int px = std::clamp(static_cast<int>(wrappedU * tex.width), 0, tex.width - 1);
    int py = std::clamp(static_cast<int>(wrappedV * tex.height), 0, tex.height - 1);

    size_t idx = (static_cast<size_t>(py) * tex.width + px) * 4;
    return glm::vec3( 
            tex.pixels[idx + 0] / 255.0f,
            tex.pixels[idx + 1] / 255.0f,
            tex.pixels[idx + 2] / 255.0f
    );
}
static uint8_t Quantize5BitTo8(uint8_t c)
{
    uint8_t q5 = static_cast<uint8_t>(std::min(c + 4, 255) >> 3); // 0..31 rounded
    return static_cast<uint8_t>((q5 << 3) | (q5 >> 2));           // 0..255 full range
}

VoxelMesh Voxelize(MeshData data)
{
    VoxelMesh vMesh;
    size_t voxels = 0;

    constexpr size_t GRID_SIZE = IceBrick::Renderer::GRID_SIZE;
    
    struct VoxelAccum {
        glm::vec3 colorSum = glm::vec3(0.0f);
        uint32_t count = 0;
    };
    std::vector<VoxelAccum> accum(GRID_SIZE * GRID_SIZE * GRID_SIZE);
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
        glm::ivec3 triMax = glm::floor(glm::max(glm::max(P0,P1),P2));

        triMin = glm::clamp(triMin, glm::ivec3(0), glm::ivec3(GRID_SIZE - 1));
        triMax = glm::clamp(triMax, glm::ivec3(0), glm::ivec3(GRID_SIZE - 1));

        glm::vec3 bV0 = E0;
        glm::vec3 bV1 = -E2;
        float d00 = glm::dot(bV0, bV0);
        float d01 = glm::dot(bV0, bV1);
        float d11 = glm::dot(bV1, bV1);

        float baryDenom = d00 * d11 - d01 * d01;
        float invBaryDenom = (std::abs(baryDenom) > 1e-8f) ? (1.0f / baryDenom) : 0.0f;

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
                    
                    glm::vec3 bary(1.0f / 3.0f);
                    if(invBaryDenom != 0.0f)
                    {
                        glm::vec3 bV2 = voxelCenter - P0;
                        float d20 = glm::dot(bV2, bV0);
                        float d21 = glm::dot(bV2, bV1);

                        float v = (d11 * d20 - d01 * d21) * invBaryDenom;
                        float w = (d00 * d21 - d01 * d20) * invBaryDenom;
                        float u = 1.0f - v - w;

                        u = std::max(u, 0.0f);
                        v = std::max(v, 0.0f);
                        w = std::max(w, 0.0f);

                        float sum = u + v + w;
                        if(sum > 1e-8f)
                            bary = glm::vec3(u, v, w) / sum;
                    }

                    glm::vec2 uv = bary.x * V0.uv + bary.y * V1.uv + bary.z * V2.uv;

                    glm::vec3 finalColor = SampleTexture(data.texture, uv);
                    
                    
                    size_t flatIndex = static_cast<size_t>(z) * (GRID_SIZE * GRID_SIZE) + static_cast<size_t>(y) * (GRID_SIZE) + static_cast<size_t>(x);

                    accum[flatIndex].colorSum += finalColor;
                    accum[flatIndex].count++;
                    voxels++;
                }
            }
        }
    }

    for(size_t i = 0; i < accum.size(); i++)
    {
        if(accum[i].count == 0) continue;

        glm::vec3 avgColor = accum[i].colorSum / static_cast<float>(accum[i].count);

        uint8_t rCol = static_cast<uint8_t>(glm::clamp(avgColor.r * 255.0f, 0.0f, 255.0f));
        uint8_t gCol = static_cast<uint8_t>(glm::clamp(avgColor.g * 255.0f, 0.0f, 255.0f));
        uint8_t bCol = static_cast<uint8_t>(glm::clamp(avgColor.b * 255.0f, 0.0f, 255.0f));
    
        rCol = Quantize5BitTo8(rCol);
        gCol = Quantize5BitTo8(gCol);
        bCol = Quantize5BitTo8(bCol);

        vMesh.VoxelMesh[i].data = IceBrick::Renderer::Voxel::Pack(rCol, gCol, bCol, 1);
    }

    spdlog::info("Successfully voxelized mesh. {}",voxels);

    GenerateSupercoverMips(vMesh, GRID_SIZE, 8);

    return vMesh;
}
