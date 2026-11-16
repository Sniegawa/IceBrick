#pragma once

#include <cstdint>
namespace IceBrick::Renderer
{
    struct Voxel
    {
        uint32_t data;

        static constexpr uint32_t Pack(uint8_t r, uint8_t g, uint8_t b, uint8_t matFlags = 0)
        {
            return (
                    static_cast<uint32_t>(r) << 24 |
                    static_cast<uint32_t>(g) << 16 |
                    static_cast<uint32_t>(b) << 8 |
                    static_cast<uint32_t>(matFlags)
            );
        }

        enum Flags : uint8_t
        {
            None = 0,
            Solid = 1 << 0,
            Emissive = 1 << 1, // Not implemented
            Transparent = 1 << 2 // Not implemented
        };
    };

    static_assert(sizeof(Voxel) == 4);

};
