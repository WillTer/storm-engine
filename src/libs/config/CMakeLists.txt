storm_lib(
    NAME config
    LINK_DEPENDENCIES
        core
    HEADER_DEPENDENCIES
        toml11::toml11
)

storm_headers(
    NAME config_iface
    HEADER_DEPENDENCIES
        toml11::toml11
)
