set(SYSTEM_DEPS_LINUX ffi)

storm_exe(
    NAME engine
    SOURCES_DEPS
        SDL3-storm

        # Modules
        animals
        animation
        ball_splash
        battle_interface
        blade
        blot
        collide
        config
        core
        diagnostics
        dialog
        filesystem
        geometry
        input
        island
        lighter
        location
        locator
        mast
        math
        model
        particles
        pcs_controls
        renderer
        rigging
        sailors
        script_library
        sea
        sea_ai
        sea_cameras
        sea_creatures
        sea_foam
        sea_operator
        shadow
        shared_headers
        ship
        sink_effect
        sound
        sound_service
        steam_api
        teleport
        tornado
        touch
        util
        water_rings
        weather
        window
        worldmap
        xinterface
)

add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/libs/shared_headers" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shared"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/src/techniques" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/techniques"
)
