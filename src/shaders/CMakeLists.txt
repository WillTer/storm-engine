file(GLOB_RECURSE SHADER_SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.slang
)

set(SHADERS_TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shaders/")

set(SHADER_TARGET_EXT_DICT "spirv,spv")
if (WIN32)
    list(APPEND SHADER_TARGET_EXT_DICT "dxil,bin")
endif ()

add_custom_command(
    OUTPUT ${SHADERS_TARGET_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_TARGET_DIR}"
)

foreach(SHADER_SOURCE ${SHADER_SOURCE_FILES})
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WLE) # Name without last extension

    foreach(SHADER_TARGET_EXT ${SHADER_TARGET_EXT_DICT})
        string(REPLACE "," ";" SHADER_TARGET_EXT_LIST ${SHADER_TARGET_EXT})
        list(GET SHADER_TARGET_EXT_LIST 0 SHADER_TARGET)
        list(GET SHADER_TARGET_EXT_LIST 1 SHADER_EXTENSION)

        # Vertex shader
        set(OUTPUT_NAME_VS "${SHADERS_TARGET_DIR}/${SHADER_NAME}_vs.${SHADER_EXTENSION}")
        add_custom_command(
            OUTPUT ${OUTPUT_NAME_VS}
            COMMAND ${SLANG_TOOL} ${SHADER_SOURCE} -entry main_vs -target ${SHADER_TARGET} -profile vs_6_0 -o ${OUTPUT_NAME_VS}
            DEPENDS ${SHADER_SOURCE})
        list(APPEND SHADER_BINARY_FILES ${OUTPUT_NAME_VS})

        # Fragment shader
        set(OUTPUT_NAME_PS "${SHADERS_TARGET_DIR}/${SHADER_NAME}_fs.${SHADER_EXTENSION}")
        add_custom_command(
            OUTPUT ${OUTPUT_NAME_PS}
            COMMAND ${SLANG_TOOL} ${SHADER_SOURCE} -entry main_fs -target ${SHADER_TARGET} -profile ps_6_0 -o ${OUTPUT_NAME_PS}
            DEPENDS ${SHADER_SOURCE})
        list(APPEND SHADER_BINARY_FILES ${OUTPUT_NAME_PS})
    endforeach(SHADER_TARGET_EXT ${SHADER_TARGET_EXT_DICT})
endforeach()

add_custom_target(shaders ALL
    DEPENDS ${SHADERS_TARGET_DIR} ${SHADER_BINARY_FILES}
)
