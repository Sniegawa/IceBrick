cmake_minimum_required(VERSION 3.24)
add_executable(IceBrickExample)

target_link_libraries(IceBrickExample PRIVATE
    IceBrickEngine
)
target_sources(IceBrickExample PRIVATE
    src/main.cpp
    src/Camera.cpp
)

if(WIN32)
    target_link_libraries(IceBrickExample PRIVATE opengl32)
else()
    target_link_libraries(IceBrickExample PRIVATE GL)
endif()


