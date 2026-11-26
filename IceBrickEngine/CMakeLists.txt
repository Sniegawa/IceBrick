cmake_minimum_required(VERSION 3.24)
add_library(IceBrickEngine STATIC)

option(ICEBRICK_VULKAN "Use Vulkan backend instead of OpenGL 4.6" OFF)

if(ICEBRICK_VULKAN)
    message(STATUS "IceBrick RHI: Compiling Vulkan 1.3 Backend")
    
    #target_sources()
else()
    message(STATUS "IceBrick RHI: Compiling OpenGL 4.6 Backend")

    target_sources(IceBrickEngine PRIVATE
        Vendor/GLAD/src/gl.c
        Vendor/OpenGL/rhi_gl.cpp
        Vendor/OpenGL/rhi_device_gl.cpp
    )
    target_include_directories(IceBrickEngine PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/GLAD/include/)
endif()

target_sources(IceBrickEngine PRIVATE
    Core/Window.cpp
    Core/WindowInput.cpp
    Core/InputManager.cpp
    Core/IOHelper.cpp
    Renderer/VoxelRenderer.cpp
    Renderer/VoxelModifier.cpp
)

target_include_directories(IceBrickEngine
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(IceBrickEngine PUBLIC
    glfw
    glm::glm
    spdlog::spdlog
    imgui
)

target_compile_definitions(IceBrickEngine PUBLIC
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    GLM_ENABLE_EXPERIMENTAL
)
