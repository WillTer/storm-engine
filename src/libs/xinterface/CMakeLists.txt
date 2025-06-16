storm_lib(
    NAME xinterface
    LINK_DEPENDENCIES
        core
        renderer
        SDL3::SDL3
        spdlog::spdlog

        $<$<PLATFORM_ID:Windows>:amstrmid ddraw>
)
