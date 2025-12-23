storm_lib(
    NAME renderer
    SOURCES_DEPS
        config
        core
        SDL3-storm
        spdlog::spdlog

        $<$<PLATFORM_ID:Windows>:legacy_stdio_definitions>
    HEADERS_DEPS
        $<$<PLATFORM_ID:Windows>:${SDK_D3D9_LIBS}>
        $<$<PLATFORM_ID:Linux>:${NATIVE_D3D9_LIBS}>
)
