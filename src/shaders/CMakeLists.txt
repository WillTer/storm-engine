file(GLOB_RECURSE SHADER_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.slang
)

set(SHADERS_TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shaders/")

set(SHADER_TARGET_EXTENSION_LIST "spv")
if (WIN32)
    list(APPEND SHADER_TARGET_EXTENSION_LIST "dxil")
endif ()

add_custom_command(
    OUTPUT ${SHADERS_TARGET_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_TARGET_DIR}"
)

foreach(SHADER_SOURCE ${SHADER_SOURCE_FILES})
    set(SHADER_ABSOLUTE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE})
    get_filename_component(SHADER_RELATIVE_PATH ${SHADER_SOURCE} DIRECTORY) # Name without last extension
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WLE) # Name without last extension

    foreach(SHADER_EXTENSION ${SHADER_TARGET_EXTENSION_LIST})
        # Vertex shader
        set(OUTPUT_NAME_VS "${SHADERS_TARGET_DIR}/${SHADER_RELATIVE_PATH}/${SHADER_NAME}_vs.${SHADER_EXTENSION}")
        add_custom_command(
            OUTPUT ${OUTPUT_NAME_VS}
            COMMAND ${SLANG_TOOL} ${SHADER_ABSOLUTE_PATH} -entry main_vs -profile vs_6_0 -o ${OUTPUT_NAME_VS}
            DEPENDS ${SHADER_ABSOLUTE_PATH})
        list(APPEND SHADER_BINARY_FILES ${OUTPUT_NAME_VS})

        # Fragment shader
        set(OUTPUT_NAME_FS "${SHADERS_TARGET_DIR}/${SHADER_RELATIVE_PATH}/${SHADER_NAME}_fs.${SHADER_EXTENSION}")
        add_custom_command(
            OUTPUT ${OUTPUT_NAME_FS}
            COMMAND ${SLANG_TOOL} ${SHADER_ABSOLUTE_PATH} -entry main_fs -profile ps_6_0 -o ${OUTPUT_NAME_FS}
            DEPENDS ${SHADER_ABSOLUTE_PATH})
        list(APPEND SHADER_BINARY_FILES ${OUTPUT_NAME_FS})
    endforeach(SHADER_EXTENSION ${SHADER_TARGET_EXTENSION_LIST})
endforeach()

storm_headers(
    NAME shader_headers
    HEADERS_DEPS
        hlslpp
)

add_custom_target(shaders ALL
    DEPENDS ${SHADERS_TARGET_DIR} ${SHADER_BINARY_FILES} shader_headers
)
