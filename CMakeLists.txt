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
find_package(tracy CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

find_path(MINIAUDIO_INCLUDE_DIR "miniaudio.h" REQUIRED)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${MINIAUDIO_INCLUDE_DIR})
add_library(miniaudio::miniaudio ALIAS miniaudio)

add_library(FastNoiseLite INTERFACE)
target_include_directories(FastNoiseLite INTERFACE 
    "${CMAKE_SOURCE_DIR}/external/FastNoiseLite"
)

add_library(imgui STATIC
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdl3.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp"
 "src/include/Texture.h")

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

# Executable
file(GLOB_RECURSE MAGMA_SOURCES "src/*.cpp")
file(GLOB_RECURSE EXTERN_SOURCES "external/*.cpp")
file(GLOB_RECURSE SHADER_SOURCES "resources/shaders/*.vert" "resources/shaders/*.frag")
add_executable(MagmaFramework
     ${MAGMA_SOURCES}
     ${EXTERN_SOURCES}
     ${SHADER_SOURCES}
     "resources/app-icon.rc"
)


# Inject the project root directory as a definition named "MAGMA_ROOT_DIR"
target_compile_definitions(MagmaFramework PUBLIC "MAGMA_ROOT_DIR=\"${CMAKE_SOURCE_DIR}/\"")

target_include_directories(MagmaFramework PUBLIC
    "src/include"
    "external/imgui"
)

target_include_directories(MagmaFramework PRIVATE ${enet_SOURCE_DIR}/include)

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
    Tracy::TracyClient        # CPU Profiling
    enet                      # Networking
    FastNoiseLite             # Noise Generation
)

target_compile_features(MagmaFramework PRIVATE cxx_std_20)

add_custom_command(TARGET MagmaFramework POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:MagmaFramework>/resources"
)