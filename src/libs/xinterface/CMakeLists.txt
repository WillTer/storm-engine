storm_lib(
    NAME xinterface
    LINK_DEPENDENCIES
        core
        renderer
        SDL2-storm
        spdlog::spdlog

        $<$<PLATFORM_ID:Windows>:amstrmid ddraw>
)
