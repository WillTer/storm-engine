### Set general compilation defines
set(COMPILE_DEFINITIONS
    -DUNICODE -D_UNICODE                             # Use Unicode
    -DNOMINMAX                                       # Use std::max()/std::min()
    -D_CRT_SECURE_NO_WARNINGS                        # Disable deprecation of unsafe functions
    -DSTORM_WATERMARK_FILE="${STORM_WATERMARK_FILE}" # Set watermark file
    -Dspdlog_EXPORTS

    $<$<BOOL:${STORM_ENABLE_STEAM}>:-DSTORM_ENABLE_STEAM=1>                 # Enable steam support if requested
    $<$<BOOL:${STORM_ENABLE_CRASH_REPORTS}>:-DSTORM_ENABLE_CRASH_REPORTS=1> # Enable crash reports if requested
    $<$<BOOL:${STORM_ENABLE_SAFE_MODE}>:-DSTORM_ENABLE_SAFE_MODE=1>         # Enable safe mode if requested
    $<$<AND:$<CONFIG:Debug>,$<PLATFORM_ID:LINUX>>:-D_DEBUG>         # Add _DEBUG define for Linux systems in Debug-mode
)

### Set general compilation flags
# Always enable debug symbols for MSVC builds
set(MSVC_CXX_FLAGS_ANY /Zi /fp:fast /utf-8)
set(MSVC_CXX_FLAGS_DEBUG /Od)
set(MSVC_CXX_FLAGS_RELEASE /O2)

set(GNU_CXX_FLAGS_ANY)
set(GNU_CXX_FLAGS_DEBUG -g -O0)
set(GNU_CXX_FLAGS_RELEASE -O3)

set(MSVC_CXX_FLAGS_WARNINGS /WX /W2)
set(GNU_CXX_FLAGS_WARNINGS -Wall -Werror=return-type -Werror=uninitialized -Werror=address)

set(MSVC_CXX_FLAGS_WARNINGS_FULL /WX /W4)
set(GNU_CXX_FLAGS_WARNINGS_FULL -Wall -Wextra -Wpedantic -Werror)

set(MSVC_CXX_FLAGS
    ${MSVC_CXX_FLAGS_ANY}
    ${MSVC_CXX_FLAGS_WARNINGS}
    $<IF:$<CONFIG:Debug>,${MSVC_CXX_FLAGS_DEBUG},${MSVC_CXX_FLAGS_RELEASE}>
)

set(GNU_CXX_FLAGS
    ${GNU_CXX_FLAGS_ANY}
    ${GNU_CXX_FLAGS_WARNINGS}
    $<IF:$<CONFIG:Debug>,${GNU_CXX_FLAGS_DEBUG},${GNU_CXX_FLAGS_RELEASE}>
)

set(STORM_CXX_FLAGS
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_CXX_FLAGS}>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${GNU_CXX_FLAGS}>
)

set(STORM_CXX_WARNINGS_FULL
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_CXX_FLAGS_WARNINGS_FULL}>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${GNU_CXX_FLAGS_WARNINGS_FULL}>
)

### Set general linker flags
set(MSVC_LINK_FLAGS
    /debug
    $<$<CONFIG:Release>:/OPT:REF /OPT:ICF /LTCG>
)

set(GNU_LINK_FLAGS)

set(STORM_LINK_FLAGS
    $<$<CXX_COMPILER_ID:MSVC>:${MSVC_LINK_FLAGS}>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${GNU_LINK_FLAGS}>
)

function(storm_exe)
    set(options FULL_WARNINGS)
    set(oneValueArgs NAME)
    set(multiValueArgs
        ACTION_DEPENDENCIES
        LINK_DEPENDENCIES
        ADDITIONAL_INCLUDE_DIRS
    )
    cmake_parse_arguments(EXE_TARGET "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/*.h
        ${CMAKE_CURRENT_LIST_DIR}/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/*.rc
    )

    add_executable(${EXE_TARGET_NAME} ${SOURCES})
    target_compile_options(${EXE_TARGET_NAME} PRIVATE ${STORM_CXX_FLAGS})
    target_compile_definitions(${EXE_TARGET_NAME} PRIVATE ${COMPILE_DEFINITIONS})
    target_link_options(${EXE_TARGET_NAME} PRIVATE ${STORM_LINK_FLAGS})
    target_link_libraries(${EXE_TARGET_NAME} PRIVATE ${EXE_TARGET_LINK_DEPENDENCIES})

    if (${EXE_TARGET_ACTION_DEPENDENCIES})
        add_dependencies(${EXE_TARGET_NAME}
            ${EXE_TARGET_ACTION_DEPENDENCIES}
        )
    endif()

    if (STORM_USE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        # ASAN
        target_compile_options(${EXE_TARGET_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
        target_link_options(${EXE_TARGET_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
    endif()

    if (${EXE_TARGET_FULL_WARNINGS})
        target_compile_options(${EXE_TARGET_NAME} PRIVATE ${STORM_CXX_WARNINGS_FULL})
    endif ()

    # Always add root src directory to includes
    target_include_directories(${EXE_TARGET_NAME}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${EXE_TARGET_ADDITIONAL_INCLUDE_DIRS}
    )
endfunction(storm_exe)

function(storm_lib)
    set(options FULL_WARNINGS)
    set(oneValueArgs NAME TYPE)
    set(multiValueArgs 
        HEADER_DEPENDENCIES
        LINK_DEPENDENCIES
        ADDITIONAL_INCLUDE_DIRS
    )
    cmake_parse_arguments(LIB_TARGET "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/*.h
        ${CMAKE_CURRENT_LIST_DIR}/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/*.rc
    )

    if (NOT DEFINED LIB_TARGET_TYPE)
        set(LIB_TARGET_TYPE STATIC)
    endif()

    add_library(${LIB_TARGET_NAME} ${LIB_TARGET_TYPE})
    target_sources(${LIB_TARGET_NAME} PRIVATE ${SOURCES})
    target_compile_options(${LIB_TARGET_NAME} PRIVATE ${STORM_CXX_FLAGS})
    target_compile_definitions(${LIB_TARGET_NAME} PRIVATE ${COMPILE_DEFINITIONS})
    target_link_options(${LIB_TARGET_NAME} PRIVATE ${STORM_LINK_FLAGS})

    target_link_libraries(${LIB_TARGET_NAME}
    PRIVATE
        ${LIB_TARGET_LINK_DEPENDENCIES}
    PUBLIC
        ${LIB_TARGET_HEADER_DEPENDENCIES}
    )

    if(STORM_USE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        # ASAN
        target_compile_options(${LIB_TARGET_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
        target_link_options(${LIB_TARGET_NAME} PRIVATE -fno-omit-frame-pointer -fsanitize=address)
    endif()

    if (${LIB_TARGET_FULL_WARNINGS})
        target_compile_options(${LIB_TARGET_NAME} PRIVATE ${STORM_CXX_WARNINGS_FULL})
    endif ()

    # Always add root src directory to includes
    target_include_directories(${LIB_TARGET_NAME}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${LIB_TARGET_ADDITIONAL_INCLUDE_DIRS}
    )
endfunction(storm_lib)

function(storm_headers)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs HEADER_DEPENDENCIES ADDITIONAL_INCLUDE_DIRS)
    cmake_parse_arguments(HEADER_TARGET "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/*.h
        ${CMAKE_CURRENT_LIST_DIR}/*.hpp
    )

    add_library(${HEADER_TARGET_NAME} INTERFACE)
    target_sources(${HEADER_TARGET_NAME} PUBLIC ${HEADERS})
    target_compile_definitions(${HEADER_TARGET_NAME} INTERFACE ${COMPILE_DEFINITIONS})

    target_link_libraries(${HEADER_TARGET_NAME} INTERFACE ${HEADER_TARGET_HEADER_DEPENDENCIES})
    set_target_properties(${HEADER_TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    # Always add root src directory to includes
    target_include_directories(${HEADER_TARGET_NAME}
        INTERFACE
            ${PROJECT_SOURCE_DIR}/src
            ${HEADER_TARGET_ADDITIONAL_INCLUDE_DIRS}
    )
endfunction(storm_headers)
