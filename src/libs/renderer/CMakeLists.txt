storm_lib(
    NAME renderer
    LINK_DEPENDENCIES
        config
        core
        SDL3::SDL3
        spdlog::spdlog

        $<$<PLATFORM_ID:Windows>:legacy_stdio_definitions>
    HEADER_DEPENDENCIES
        $<$<PLATFORM_ID:Windows>:${SDK_D3D9_LIBS}>
        $<$<PLATFORM_ID:Linux>:${NATIVE_D3D9_LIBS}>
)
