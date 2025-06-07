include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         v3.8.1
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    fast_float
    GIT_REPOSITORY  https://github.com/fastfloat/fast_float.git
    GIT_TAG         v8.0.2
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG         release-2.32.6
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    sentry
    GIT_REPOSITORY  https://github.com/getsentry/sentry-native.git
    GIT_TAG         0.8.5
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog.git
    GIT_TAG         v1.15.3
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY  https://github.com/madler/zlib.git
    GIT_TAG         v1.3.1
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
    GIT_TAG         11.2.0
    GIT_SHALLOW     ON
)

FetchContent_Declare(
    toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG        v4.4.0
)

FetchContent_Declare(
    storm-audio
    GIT_REPOSITORY  https://github.com/WillTer/storm-audio.git
    GIT_TAG         3e23f57f6f726813cb3a195d683cb3e31b425d07
    GIT_SHALLOW     ON
)

FetchContent_MakeAvailable(Catch2 fast_float sentry spdlog fmt toml11 storm-audio)
 
if (WIN32)
    FetchContent_MakeAvailable(SDL2 zlib)

    include(cmake/directxsdk.cmake)
elseif(LINUX)
    # On Linux use SDL2 and zlib from package manager
    find_package(SDL2 REQUIRED)
    find_package(ZLIB REQUIRED)
    include(cmake/linux_d3d9.cmake)
endif()

add_library(SDL2-storm INTERFACE)
target_link_libraries(SDL2-storm
    INTERFACE
        $<$<PLATFORM_ID:Windows>:SDL2::SDL2-static>
        $<$<PLATFORM_ID:Linux>:${SDL2_LIBRARIES}>
)
target_include_directories(SDL2-storm
    INTERFACE
        $<$<PLATFORM_ID:Linux>:${SDL2_INCLUDE_DIRS}>
)

add_library(Zlib-storm INTERFACE)
target_link_libraries(Zlib-storm
    INTERFACE
        $<$<PLATFORM_ID:Windows>:zlib>
        $<$<PLATFORM_ID:Linux>:${ZLIB_LIBRARIES}>
)
target_include_directories(Zlib-storm
    INTERFACE
        $<$<PLATFORM_ID:Linux>:${ZLIB_INCLUDE_DIRS}>
)
