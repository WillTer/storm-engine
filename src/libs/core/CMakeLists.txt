storm_lib(
    NAME core
    SOURCES_DEPS
        config_iface
        diagnostics
        FastFloat::fast_float
        SDL3-storm
        spdlog::spdlog
        Zlib-storm

        $<$<PLATFORM_ID:Windows>:comctl32>
    HEADERS_DEPS
        EnTT::EnTT
)
