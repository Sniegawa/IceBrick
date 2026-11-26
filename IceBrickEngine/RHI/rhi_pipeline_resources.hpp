#pragma once

#include "RHI/rhi_device.hpp"
#include "RHI/rhi_handles.hpp"
#include "RHI/rhi_types.hpp"
#include <cstdint>
#include <array>
#include <set>

namespace IceBrick::RHI
{
    class PipelineResources
    {
    public:
        PipelineResources(uint32_t setIndex = 0) : m_SetIndex(setIndex) {}

        enum class ResourceType : uint8_t
        {
            None,
            UniformBuffer,
            StorageBuffer,
            Texture,
            StorageImage
        };

        struct BindingSlot
        {
            uint64_t handleID = 0;
            ResourceType type = ResourceType::None;
            ImageAccess access = ImageAccess::ReadWrite; // Only used for type StorageImage
            uint32_t mipLevel = 0; // Only used for StorageImage
        };
        
        void SetStorageBuffer(uint32_t binding, BufferHandle handle)
        {
            m_Bindings[binding] = { handle.id, ResourceType::StorageBuffer };
        }

        void SetUniformBuffer(uint32_t binding, BufferHandle handle)
        {
            m_Bindings[binding] = { handle.id, ResourceType::UniformBuffer };
        }


        void SetTexture(uint32_t binding, TextureHandle handle)
        {
            m_Bindings[binding] = {handle.id, ResourceType::Texture };
        }

        void SetStorageImage(uint32_t binding, TextureHandle handle, ImageAccess access = ImageAccess::ReadWrite, uint32_t mip = 0)
        {
            m_Bindings[binding] = {handle.id, ResourceType::StorageImage, access, mip};
        }


        void Commit(RenderDevice* device) const
        {
            for(uint32_t binding = 0; binding < MAX_BINDINGS; ++binding)
            {
                const auto& slot = m_Bindings[binding];

                if(slot.type == ResourceType::None) continue;

                if(slot.type == ResourceType::StorageBuffer)
                    device->BindStorageBuffer(m_SetIndex, binding, {slot.handleID});

                else if(slot.type == ResourceType::UniformBuffer)
                    device->BindUniformBuffer(m_SetIndex, binding, {slot.handleID});

                else if(slot.type == ResourceType::Texture)
                    device->BindTexture(m_SetIndex, binding, {slot.handleID});

                else if(slot.type == ResourceType::StorageImage)
                    device->BindImage(m_SetIndex, binding, {slot.handleID}, slot.access, slot.mipLevel);
            }
        }

    private:
        static constexpr uint32_t MAX_BINDINGS = 16; // Mirror vulkan cap
        std::array<BindingSlot, MAX_BINDINGS> m_Bindings{};
        uint32_t m_SetIndex = 0;
    };
};
