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

add_executable(cg_opengl
    src/main.cpp
    external/glad-build/src/glad.c
)

# Includes (GLAD gerado + GLFW + GLM header-only)
target_include_directories(cg_opengl PRIVATE
    external/glad-build/include
    external/glfw/include
    external/glm
)

# Link GLFW (puxa opengl32 no Windows automaticamente via target)
target_link_libraries(cg_opengl PRIVATE glfw)

if (WIN32)
    target_compile_definitions(cg_opengl PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Warnings
if (MSVC)
    target_compile_options(cg_opengl PRIVATE /W4 /permissive-)
else()
    target_compile_options(cg_opengl PRIVATE -Wall -Wextra -Wpedantic)
endif()
