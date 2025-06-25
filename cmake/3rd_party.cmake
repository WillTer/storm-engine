include(cmake/slang.cmake)
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
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_Declare(
    storm-audio
    GIT_REPOSITORY  https://github.com/WillTer/storm-audio.git
    GIT_TAG         120dcd27a60c73533760124abfa264e2fdbd8276
)

set(BUILD_SHARED_LIBS ON)

if (WIN32)
    FetchContent_MakeAvailable(SDL3 zlib)
elseif(LINUX)
    # On Linux use SDL3 and zlib from package manager
    find_package(SDL3 REQUIRED)
    find_package(ZLIB REQUIRED)
endif()

FetchContent_MakeAvailable(Catch2 fast_float spdlog entt glm storm-audio)

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
