storm_lib(
    NAME core
    SOURCES_DEPS
        config_headers
        diagnostics
        FastFloat::fast_float
        hlslpp
        SDL3-storm
        spdlog::spdlog
        Zlib-storm
        shader_headers

        $<$<PLATFORM_ID:Windows>:comctl32>
    HEADERS_DEPS
        EnTT::EnTT
)
