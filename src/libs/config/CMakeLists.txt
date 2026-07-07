storm_lib(
    NAME config
    SOURCES_DEPS
        asset_server
        core
        spdlog::spdlog
    HEADERS_DEPS
        hlslpp
        shader_headers
)
