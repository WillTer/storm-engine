storm_lib(
    NAME xinterface
    LINK_DEPENDENCIES
        core
        renderer
        SDL3-storm
        spdlog::spdlog

        $<$<PLATFORM_ID:Windows>:amstrmid ddraw>
)
