storm_lib(
    NAME core
    LINK_DEPENDENCIES
        config_iface
        diagnostics
        FastFloat::fast_float
        SDL2-storm
        spdlog::spdlog
        Zlib-storm

        $<$<PLATFORM_ID:Windows>:comctl32>
    HEADER_DEPENDENCIES
        EnTT::EnTT
)
