cmake_minimum_required(VERSION 3.18)

project(storm-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(STORM_ENABLE_CRASH_REPORTS "Enable automatic crash reports" OFF)
option(STORM_ENABLE_STEAM "Enable Steam integration" OFF)
option(STORM_ENABLE_SAFE_MODE "Enable additional runtime checks" OFF)
option(STORM_USE_ASAN "Enable address sanitizer" OFF) # For future
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
include(storm_target)

# Get git commit for watermark
execute_process(COMMAND
    git describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE STORM_BUILD_WATERMARK
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("STORM_BUILD_WATERMARK=${STORM_BUILD_WATERMARK}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/watermark.hpp.in" "${STORM_WATERMARK_FILE}" @ONLY)

### Set up third-party dependencies
include(3rd_party)

add_subdirectory(src)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT engine)
