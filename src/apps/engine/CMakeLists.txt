if (WIN32)
set(SYSTEM_DEPS
    "comctl32"
    "dbghelp"
    "winhttp"
)
else()
set(SYSTEM_DEPS "ffi")
endif()

STORM_SETUP(
    TARGET_NAME engine
    TYPE executable
    DEPENDENCIES

    # internal
    core
    diagnostics
    util
    window

    # external
    sentry::sentry
    ${SDL2_LIBRARIES}
    zlib

    # system
    ${SYSTEM_DEPS}
)

add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/libs/shared_headers/include/shared" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shared"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/techniques" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/techniques"
)
