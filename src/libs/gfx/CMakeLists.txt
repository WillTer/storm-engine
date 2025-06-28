storm_lib(
    NAME gfx
    FULL_WARNINGS
    SOURCES_DEPS
        asset_server
        config
        core
        SDL3-storm
        spdlog::spdlog
    HEADERS_DEPS
        hlslpp
)
