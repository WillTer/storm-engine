include(FetchContent)

FetchContent_Declare(
    directxsdk
    URL         https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe
    URL_HASH SHA512=24e1e9bda319b780124b865f4640822cfc44e4d18fbdcc8456d48fe54081652ce4ddb63d3bd8596351057cbae50fc824b8297e99f0f7c97547153162562ba73f
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(directxsdk)

function(import_directx_lib)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs)
    cmake_parse_arguments(DX_LIB "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )
    add_library(${DX_LIB_NAME} SHARED IMPORTED)
    set_property(TARGET ${DX_LIB_NAME} PROPERTY
        IMPORTED_IMPLIB "${directxsdk_SOURCE_DIR}/Lib/x64/${DX_LIB_NAME}.lib"
    )
    target_include_directories(${DX_LIB_NAME}
    INTERFACE
        "${directxsdk_SOURCE_DIR}/Include"
    )
endfunction()

import_directx_lib(NAME d3d9)
import_directx_lib(NAME d3dx9)
import_directx_lib(NAME DxErr)

set(SDK_D3D9_LIBS
    d3d9
    d3dx9
    DxErr
)
