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
    SDL3
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG         release-3.2.16
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
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.15.0
    GIT_SHALLOW    ON
)

FetchContent_Declare(
    storm-audio
    GIT_REPOSITORY  https://github.com/WillTer/storm-audio.git
    GIT_TAG         3f3a4a0cddce04cd65f050a4c67ea38b082d135c
)

FetchContent_MakeAvailable(Catch2 fast_float sentry spdlog entt storm-audio)

if (WIN32)
    FetchContent_MakeAvailable(SDL3 zlib)

    include(cmake/directxsdk.cmake)
elseif(LINUX)
    # On Linux use SDL3 and zlib from package manager
    find_package(SDL3 REQUIRED)
    find_package(ZLIB REQUIRED)
    include(cmake/linux_d3d9.cmake)
endif()

add_library(SDL3-storm INTERFACE)
target_link_libraries(SDL3-storm
    INTERFACE
        $<$<PLATFORM_ID:Windows>:SDL3::SDL3>
        $<$<PLATFORM_ID:Linux>:${SDL3_LIBRARIES}>
)
target_include_directories(SDL3-storm
    INTERFACE
        $<$<PLATFORM_ID:Linux>:${SDL3_INCLUDE_DIRS}>
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
