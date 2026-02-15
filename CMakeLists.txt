cmake_minimum_required(VERSION 3.20)

project(MagmaFramework)

# Dependencies
find_package(glm CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(freeimage CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_library(glad ALIAS glad::glad)
add_library(glm ALIAS glm::glm)

find_path(MINIAUDIO_INCLUDE_DIR "miniaudio.h" REQUIRED)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${MINIAUDIO_INCLUDE_DIR})
add_library(miniaudio::miniaudio ALIAS miniaudio)

add_library(FastNoiseLite INTERFACE)
target_include_directories(FastNoiseLite INTERFACE 
    "${CMAKE_SOURCE_DIR}/external/FastNoiseLite"
)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE 
    "${CMAKE_SOURCE_DIR}/external/stb"
)

add_library(imgui STATIC
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdl3.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
    "${CMAKE_SOURCE_DIR}/external/imgui"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends"
)

target_include_directories(imgui PRIVATE "${CMAKE_SOURCE_DIR}/external/imgui/backends")

target_link_libraries(imgui PUBLIC SDL3::SDL3 OpenGL::GL)

include(FetchContent)
FetchContent_Declare(
    enet
    GIT_REPOSITORY https://github.com/lsalzman/enet.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(enet)

add_subdirectory("external/glui")

# Executable
file(GLOB_RECURSE MAGMA_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS "resources/shaders/*.vert" "resources/shaders/*.frag")

set(TRACY_SOURCE "${CMAKE_SOURCE_DIR}/external/tracy/public/TracyClient.cpp")

add_executable(MagmaFramework
     ${MAGMA_SOURCES}
     ${SHADER_SOURCES}
     "resources/app-icon.rc")

add_compile_definitions(WIN32_LEAN_AND_MEAN)

# Inject the project root directory as a definition named "MAGMA_ROOT_DIR"
target_compile_definitions(MagmaFramework PUBLIC "MAGMA_ROOT_DIR=\"${CMAKE_SOURCE_DIR}/\"")

target_include_directories(MagmaFramework PUBLIC
    "src/include"
    "external/imgui"
)

target_include_directories(MagmaFramework PRIVATE ${enet_SOURCE_DIR}/include)


option(TRACY_ENABLE "Enable Tracy Profiler" ON)
option(TRACY_ON_DEMAND "Enable profiling on demand" ON)
target_include_directories(MagmaFramework PUBLIC 
    "${CMAKE_SOURCE_DIR}/external/tracy/public"
)

target_sources(MagmaFramework PRIVATE 
    "${CMAKE_SOURCE_DIR}/external/tracy/public/TracyClient.cpp"
)

if(TRACY_ENABLE)
    target_compile_definitions(MagmaFramework PUBLIC TRACY_ENABLE)
    if(TRACY_ON_DEMAND)
        target_compile_definitions(MagmaFramework PUBLIC TRACY_ON_DEMAND)
    endif()
endif()

# Link libraries
target_link_libraries(MagmaFramework PUBLIC
    glm::glm                  # Math
    SDL3::SDL3                # Windowing/Input
    glad::glad                # OpenGL Loader
    fmt::fmt                  # String Formatting
    freetype                  # Font Rendering (Often not namespaced in vcpkg)
    assimp::assimp            # 3D Model Loading
    freeimage::FreeImage      # Image Loading
    miniaudio::miniaudio      # Audio

    enet                      # Networking
    FastNoiseLite             # Noise Generation
    glui
    stb

    ws2_32      # Required by Tracy
    dbghelp     # Required by Tracy
    advapi32    # Required by Tracy
    user32      # Required by Tracy
)

find_package(Threads REQUIRED)
target_link_libraries(MagmaFramework PUBLIC Threads::Threads)


target_compile_features(MagmaFramework PRIVATE cxx_std_20)

add_custom_command(TARGET MagmaFramework POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:MagmaFramework>/resources"
)