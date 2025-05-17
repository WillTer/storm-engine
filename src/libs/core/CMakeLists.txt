STORM_SETUP(
    TARGET_NAME core
    TYPE library
    DEPENDENCIES diagnostics math shared_headers steam_api FastFloat::fast_float ${SDL2_LIBRARIES} window fmt zlib
)
