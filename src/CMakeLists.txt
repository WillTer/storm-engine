set(SYSTEM_DEPS_WIN
    amstrmid
    comctl32
    dbghelp
    ddraw
    legacy_stdio_definitions
    winhttp
)

set(SYSTEM_DEPS_LINUX ffi)

storm_exe(
    NAME engine
    LINK_DEPENDENCIES
        FastFloat::fast_float
        fmt
        sentry::sentry
        spdlog::spdlog
        SDL2-storm
        Zlib-storm
        OpenAL
        $<$<PLATFORM_ID:Windows>:${SDK_D3D9_LIBS} ${SYSTEM_DEPS_WIN}>
        $<$<PLATFORM_ID:Linux>:${NATIVE_D3D9_LIBS} ${SYSTEM_DEPS_LINUX}>
)

add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/libs/shared_headers" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shared"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/techniques" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/techniques"
)
