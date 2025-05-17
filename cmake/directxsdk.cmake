include(FetchContent)

FetchContent_Declare(
    directxsdk
    URL         https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe
    URL_HASH SHA512=24e1e9bda319b780124b865f4640822cfc44e4d18fbdcc8456d48fe54081652ce4ddb63d3bd8596351057cbae50fc824b8297e99f0f7c97547153162562ba73f
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(directxsdk)

add_library(d3d9 SHARED IMPORTED)
set_property(TARGET d3d9 PROPERTY
    IMPORTED_IMPLIB "${directxsdk_SOURCE_DIR}/Lib/x64/d3d9.lib"
)

add_library(d3dx9 SHARED IMPORTED)
set_property(TARGET d3dx9 PROPERTY
    IMPORTED_IMPLIB "${directxsdk_SOURCE_DIR}/Lib/x64/d3dx9.lib"
)

add_library(dxerr SHARED IMPORTED)
set_property(TARGET dxerr PROPERTY
    IMPORTED_IMPLIB "${directxsdk_SOURCE_DIR}/Lib/x64/DxErr.lib"
)

file(GLOB DX_HDRS CONFIGURE_DEPENDS ${directxsdk_SOURCE_DIR}/Include/*.h)
add_library(
    directx-headers INTERFACE ${DX_HDRS}
)
target_include_directories(directx-headers
INTERFACE
    "${directxsdk_SOURCE_DIR}/Include"
)

set(SDK_D3D9_LIBS
    d3d9
    d3dx9
    dxerr
    directx-headers
)
