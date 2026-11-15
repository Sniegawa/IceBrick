#pragma once

#include "RHI/TextureDescription.hpp"

#include <memory>

namespace IceBrick::RHI
{

    // Although i could include TextureDescription field here
    // and define the getters, i want this class to be an interface 
    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual const TextureDescription& getDescription() const = 0;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetDepth() const = 0;

        virtual void SetData(const void* data, uint32_t size) = 0;
        
        virtual void* GetNativeHandle() = 0;

        static std::shared_ptr<Texture> Create(const TextureDescription& description);
    };

};
