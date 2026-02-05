cmake_minimum_required(VERSION 3.5)
project(glui_library) # Renamed to ensure no conflict with "MagmaFramework"

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gl2d/CMakeLists.txt")
    add_subdirectory(thirdparty/gl2d)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb_image/CMakeLists.txt")
    add_subdirectory(thirdparty/stb_image)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb_truetype/CMakeLists.txt")
    add_subdirectory(thirdparty/stb_truetype)
endif()

add_library(glui STATIC "glui/src/glui.cpp")

target_include_directories(glui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glui/include")

target_link_libraries(glui PUBLIC
    glm         # Uses the ALIAS we made in main CMake
    glad        # Uses the ALIAS we made in main CMake
    imgui       # Uses your main ImGui target
    gl2d        # Uses the internal one we added above
    stb_image
    stb_truetype
)

target_compile_features(glui PUBLIC cxx_std_17)