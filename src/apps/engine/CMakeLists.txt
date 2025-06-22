set(SYSTEM_DEPS_LINUX ffi)

storm_exe(
    NAME engine
    ACTION_DEPENDENCIES
        shaders
    LINK_DEPENDENCIES
        SDL3-storm

        # Modules
        animals
        animation
        asset_server
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
        renderer_next
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
)
