#pragma once

#include "Renderer/Texture.hpp"
#include "Renderer/TextureDescription.hpp"
#include <memory>
namespace IceBrick::RHI
{        
    void Initialize(void* NativeWindowHandle);
    
    void ClearScreen(float r, float g, float b);
}
