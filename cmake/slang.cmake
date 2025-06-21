include(FetchContent)

set(SYS_NAME)
set(SYS_ARCH)
set(ARCHIVE_MD5)
if (WIN32)
    set(SYS_NAME "windows")
    if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "IA64")
        set(SYS_ARCH "x86_64")
        set(ARCHIVE_MD5 775adb2de3db9a5f57077c0bcdb457f6)
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "ARM64")
        set(SYS_ARCH "aarch64")
        set(ARCHIVE_MD5 8260ee26a634b0be49d60fcd92549d62)
    endif ()
elseif(LINUX)
    set(SYS_NAME "linux")
    set(SYS_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")
    if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(ARCHIVE_MD5 c91e43cb6643026e7476f4286508a530)
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
        set(ARCHIVE_MD5 d332afaff4f1142f28b79d9dad29d0b1)
    endif ()
elseif(DARWIN)
    set(SYS_NAME "macos")
    if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(SYS_ARCH "x86_64")
        set(ARCHIVE_MD5 b558aaa6b3a0004c1d98751984b0e27b)
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(SYS_ARCH "aarch64")
        set(ARCHIVE_MD5 dfce76f58d35e631508a65d267b046a8)
    endif ()
endif ()

FetchContent_Declare(
    slang
    URL                         https://github.com/shader-slang/slang/releases/download/v2025.10.4/slang-2025.10.4-${SYS_NAME}-${SYS_ARCH}.zip
    URL_HASH                    MD5=${ARCHIVE_MD5}
    DOWNLOAD_EXTRACT_TIMESTAMP  ON
)

FetchContent_MakeAvailable(slang)

set(SLANG_TOOL)

if (WIN32)
    set(SLANG_TOOL "${slang_SOURCE_DIR}/bin/slangc.exe")
else()
    set(SLANG_TOOL "${slang_SOURCE_DIR}/bin/slangc")
endif ()
