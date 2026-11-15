#pragma once

#include "RHI/Texture.hpp"
#include "RHI/TextureDescription.hpp"

namespace IceBrick::RHI
{
    class GLTexture : public Texture
    {
    public:
        GLTexture(const TextureDescription& description);

        virtual ~GLTexture() override;

        virtual const TextureDescription& getDescription() const override;

        virtual uint32_t GetWidth() const override;
        virtual uint32_t GetHeight() const override;
        virtual uint32_t GetDepth() const override;

        virtual void SetData(const void* data, uint32_t size) override;

        virtual void* GetNativeHandle() override;

    private:
        TextureDescription m_Description;

        unsigned int m_ID;
    };

};
