cmake_minimum_required(VERSION 3.11)

project(SoftRender)

option(DEFAULT_DEBUG "Set Debug as default build type" OFF)

if(NOT CMAKE_BUILD_TYPE AND DEFAULT_DEBUG)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)
find_library(OPENGL_FRAMEWORK OpenGL) # OpenGL 用来绘制 Imgui

include_directories(${PROJECT_SOURCE_DIR}/external)
include_directories(${PROJECT_SOURCE_DIR}/external/tinyobjloader)
include_directories(${PROJECT_SOURCE_DIR}/external/imgui)
include_directories(${PROJECT_SOURCE_DIR}/external/imgui/imgui-sfml)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(/opt/homebrew/Cellar/sfml/3.0.2/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/external/imgui SRC_IMGUI)
aux_source_directory(${PROJECT_SOURCE_DIR}/external/imgui/imgui-sfml SRC_IMGUI_SFML)

add_executable(SoftRender main.cpp ${SRC_LIST} ${SRC_IMGUI} ${SRC_IMGUI_SFML})

target_precompile_headers(SoftRender PRIVATE ${PROJECT_SOURCE_DIR}/include/preheader.h)
target_link_libraries(SoftRender PRIVATE SFML::Graphics SFML::Window SFML::System ${OPENGL_FRAMEWORK})