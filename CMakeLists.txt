cmake_minimum_required(VERSION 3.16)
project(cg_opengl LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Desativar builds desnecessários do GLFW
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)

add_subdirectory(external/glfw)

set(CG_ENGINE_SOURCES
    src/core/Application.cpp
    src/core/Window.cpp
    src/core/FPSCounter.cpp
    src/render/Shader.cpp
    src/render/Grid.cpp
    src/render/Mesh.cpp
    src/render/Model.cpp
    src/render/ModelLoader.cpp
    src/render/Renderer.cpp
    src/render/Skybox.cpp
    src/render/Material.cpp
    src/input/Camera.cpp
    src/input/Input.cpp
    src/physics/Physics.cpp
    src/ui/DebugUI.cpp
    external/glad-build/src/glad.c
)

add_library(cg_engine STATIC ${CG_ENGINE_SOURCES})

target_include_directories(cg_engine PUBLIC
    include
    external/glad-build/include
    external/glfw/include
    external/glm
)

target_link_libraries(cg_engine PUBLIC glfw)

if (WIN32)
    target_compile_definitions(cg_engine PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Warnings
if (MSVC)
    target_compile_options(cg_engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(cg_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(cg_opengl src/main.cpp)

target_link_libraries(cg_opengl PRIVATE cg_engine)

if (MSVC)
    target_compile_options(cg_opengl PRIVATE /W4 /permissive-)
else()
    target_compile_options(cg_opengl PRIVATE -Wall -Wextra -Wpedantic)
endif()
