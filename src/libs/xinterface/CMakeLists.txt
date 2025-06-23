storm_lib(
    NAME xinterface
    SOURCES_DEPS
        core
        renderer_next
        SDL3-storm
        spdlog::spdlog

        $<$<PLATFORM_ID:Windows>:amstrmid ddraw>
)
