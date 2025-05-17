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

FetchContent_MakeAvailable(Catch2 fast_float sentry spdlog zlib fmt)

set(SDK_D3D9_LIBS)
if (WIN32)
    FetchContent_MakeAvailable(SDL2)

    set(SDL2_LIBRARIES SDL2::SDL2 SDL2::SDL2main)
    set(SDK_D3D9_LIBS
        ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/lib/d3d9.lib
        ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/lib/d3dx9.lib
        ${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/lib/DxErr.lib)
    # FIXME: Remove global include declarations
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/vcpkg_installed/x64-windows/include/directxsdk")
else()
    include(linux)
endif()
