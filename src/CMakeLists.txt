if (WIN32)
    set(SYSTEM_DEPS
        "amstrmid"
        "comctl32"
        "dbghelp"
        "ddraw"
        "legacy_stdio_definitions"
        "winhttp"
    )
else()
    set(SYSTEM_DEPS "ffi ${NATIVE_D3D9_LIBS}")
endif()

storm_exe(
    NAME engine
    LINK_DEPENDENCIES
        FastFloat::fast_float
        fmt
        sentry::sentry
        spdlog::spdlog
        zlib
        ${SDK_D3D9_LIBS}
        ${SDL2_LIBRARIES}
        ${SYSTEM_DEPS}
)

add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/libs/shared_headers" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shared"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/techniques" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/techniques"
)
