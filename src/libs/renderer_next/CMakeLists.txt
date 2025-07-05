storm_lib(
    NAME renderer_next
    FULL_WARNINGS
    SOURCES_DEPS
        asset_server
        config
        core
        spdlog::spdlog
    HEADERS_DEPS
        hlslpp
        SDL3-storm
        shader_headers
)
