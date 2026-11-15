#pragma once

#include <cstdint>

namespace IceBrick::RHI
{
    struct BufferHandle {
        uint64_t id;

        uint32_t GetIndex() const { return static_cast<uint32_t>(id & 0xFFFFFFFF); }
        uint32_t GetGeneration() const { return static_cast<uint32_t>(id >> 32); }
    };

    struct TextureHandle
    {
        uint64_t id;

        uint32_t GetIndex() const { return static_cast<uint32_t>(id & 0xFFFFFFFF); }
        uint32_t GetGeneration() const { return static_cast<uint32_t>(id >> 32); }

    };

};
