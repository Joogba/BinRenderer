cmake_minimum_required(VERSION 3.20)

project(BinRenderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vcpkg Integration
# This is usually handled by the toolchain file passed to CMake
# e.g. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake

# Find Packages
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(unofficial-spirv-reflect CONFIG REQUIRED) # vcpkg port name often differs

# Include Directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
)

# Collect Source Files
file(GLOB_RECURSE ENGINE_SOURCES
    "Core/*.cpp"
    "RHI/*.cpp"
    "RenderPass/*.cpp"
    "Rendering/*.cpp"
    "Scene/*.cpp"
    "Utils/*.cpp"
    "Platform/*.cpp"
)

# Exclude Legacy and Examples from the library
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "LegacyVulkan/.*")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX "Examples/.*")

# Create Engine Library
add_library(BinRendererLib STATIC ${ENGINE_SOURCES})

# Link Libraries
target_link_libraries(BinRendererLib PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    assimp::assimp
    imgui::imgui
    unofficial::spirv-reflect::spirv-reflect
)

# Example Executable (PBRTest_Full_RHI)
add_executable(BinRenderer_PBRTest "Examples/Ex01_Context/PBRTest_Full_RHI.cpp")
target_link_libraries(BinRenderer_PBRTest PRIVATE BinRendererLib)

# Copy Assets to Output Directory (Optional but useful)
add_custom_command(TARGET BinRenderer_PBRTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BinRenderer_PBRTest>/assets
)
