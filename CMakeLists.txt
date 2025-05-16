cmake_minimum_required(VERSION 3.18)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Path to vcpkg toolchain file")

project(storm-engine)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(StormSetup)

option(STORM_ENABLE_CRASH_REPORTS "Enable automatic crash reports" OFF)
option(STORM_ENABLE_STEAM "Enable Steam integration" OFF)
option(STORM_ENABLE_SAFE_MODE "Enable additional runtime checks" OFF)
option(STORM_USE_CONAN_SDL "Use sdl from conan" ON)
if (NOT WIN32)
    option(STORM_MESA_NINE "Use Gallium Nine from Mesa for native D3D9 API" OFF)
endif()

### Set up output paths
if(GENERATOR_IS_MULTI_CONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
endif(GENERATOR_IS_MULTI_CONFIG)

set(STORM_WATERMARK_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/watermark.hpp CACHE FILEPATH "Include file containing build revision, etc." FORCE)

if (WIN32)
    set(SDL2_LIBRARIES "$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>")
else()
    include(linux)
endif()

### Set up third-party dependencies
# TODO
find_package(FastFloat CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(sentry CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

set(SDK_D3D9_LIBS)
if (WIN32)
    set(SDK_D3D9_LIBS
        ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/lib/d3d9.lib
        ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/lib/d3dx9.lib
        ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/lib/DxErr.lib)
endif(WIN32)

### Set up build scripts
set(SRC_DIRS "src" CACHE STRING "File locations for public header files" FORCE)
set(PUBLIC_INCLUDE_DIRS "include" CACHE STRING "File locations for source files" FORCE)
set(TESTSUITE_DIRS "testsuite" CACHE STRING "File locations for tests" FORCE)
set(RESOURCE_DIRS "rsrc" CACHE STRING "File locations for tests" FORCE)

set(SOURCE_FILE_EXTENSIONS "*.cpp" CACHE STRING "File extensions for source files" FORCE)
set(HEADER_FILE_EXTENSIONS "*.h" "*.hpp" CACHE STRING "File extensions for header files" FORCE)
###

add_subdirectory(src)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
