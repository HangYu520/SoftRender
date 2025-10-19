cmake_minimum_required(VERSION 3.11)

project(SoftRender)

option(DEFAULT_DEBUG "Set Debug as default build type" ON)

if(NOT CMAKE_BUILD_TYPE AND DEFAULT_DEBUG)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/external)
include_directories(${PROJECT_SOURCE_DIR}/external/tinyobjloader)
include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

add_executable(SoftRender main.cpp ${SRC_LIST})

target_precompile_headers(SoftRender PRIVATE ${PROJECT_SOURCE_DIR}/include/preheader.h)