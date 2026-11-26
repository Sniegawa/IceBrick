#pragma once

#include "glm/ext/vector_int3.hpp"
#include <cstdint>
namespace IceBrick::Renderer
{
    struct Voxel
    {
        uint16_t data;

        static constexpr uint16_t Pack(uint8_t r8, uint8_t g8, uint8_t b8, bool solid)
        {
            uint16_t r5 = static_cast<uint16_t>(r8 >> 3) & 0x1Fu;
            uint16_t g5 = static_cast<uint16_t>(g8 >> 3) & 0x1Fu;
            uint16_t b5 = static_cast<uint16_t>(b8 >> 3) & 0x1Fu;
            return static_cast<uint16_t>((r5 << 11) | (g5 << 6) | (b5 << 1) | (solid ? 1u : 0u));
        }
    };

    static_assert(sizeof(Voxel) == 2);

    struct alignas(16) RaycastHit
    {
        glm::ivec3 cell;
        uint32_t valid;
        glm::ivec3 normal;
        uint32_t pad;
    };
};
