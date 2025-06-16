message("Using DXVK Native for D3D9 API")

include(ExternalProject)
ExternalProject_Add(dxvk
    GIT_REPOSITORY    https://github.com/doitsujin/dxvk.git
    GIT_TAG           v2.6.2
    GIT_SHALLOW       ON
    BUILD_ALWAYS      OFF
    CONFIGURE_HANDLED_BY_BUILD ON
    CONFIGURE_COMMAND meson setup ../dxvk --buildtype=release -Denable_d3d11=false -Denable_d3d10=false -Denable_dxgi=false -Dnative_sdl2=disabled -Dnative_sdl3=enabled
    BUILD_COMMAND     ninja
    INSTALL_COMMAND   ""
)
ExternalProject_Get_property(dxvk SOURCE_DIR BINARY_DIR)
set(DXVK_NATIVE_INCLUDE_DIRS
    "${SOURCE_DIR}/include/native/directx"
    "${SOURCE_DIR}/include/native/windows"
)
set(NATIVE_D3D9_LIBS ${BINARY_DIR}/src/d3d9/libdxvk_d3d9.so)
include_directories("${DXVK_NATIVE_INCLUDE_DIRS}")
add_custom_target(dependencies ALL DEPENDS dxvk)
