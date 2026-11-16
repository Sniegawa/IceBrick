#pragma once

#include "RHI/rhi_device.hpp"
namespace IceBrick::RHI
{        
    RenderDevice* InitializeDevice(void* NativeWindowHandle);
    
    RenderDevice* GetDevice();

    void ShutdownDevice();
}
