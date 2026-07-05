set(SHADERS_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource/shaders/")

set(SHADER_TARGET_EXTENSION_LIST "spv")
if (WIN32)
    list(APPEND SHADER_TARGET_EXTENSION_LIST "dxil")
endif ()

add_custom_command(
    OUTPUT ${SHADERS_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADERS_OUTPUT_DIR}"
)

set(SHADER_META_FILES)
set(SHADER_BINARY_FILES)

function(shaders_target)
    set(options)
    set(oneValueArgs
        NAME
        DIR
        PROFILE
    )
    set(multiValueArgs)
    cmake_parse_arguments(SHADERS_TARGET "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${SHADERS_TARGET_DIR} CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/${SHADERS_TARGET_DIR}/*.slang
    )

    foreach(SHADER_SOURCE ${SOURCE_FILES})
        set(SHADER_ABSOLUTE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${SHADERS_TARGET_DIR}/${SHADER_SOURCE})
        get_filename_component(SHADER_RELATIVE_PATH ${SHADER_SOURCE} DIRECTORY) # Name without last extension
        get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME_WLE) # Name without last extension

        set(SHADER_META_ABSOLUTE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${SHADERS_TARGET_DIR}/${SHADER_RELATIVE_PATH}/${SHADER_NAME}_meta.ini)
        set(SHADER_META_OUTPUT_PATH ${SHADERS_OUTPUT_DIR}/${SHADER_RELATIVE_PATH}/${SHADER_NAME}_meta.ini)
        add_custom_command(
                OUTPUT ${SHADER_META_OUTPUT_PATH}
                COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_META_ABSOLUTE_PATH} ${SHADER_META_OUTPUT_PATH}
                DEPENDS ${SHADER_META_ABSOLUTE_PATH})
        list(APPEND SHADER_META_FILES ${SHADER_META_OUTPUT_PATH})

        foreach(SHADER_EXTENSION ${SHADER_TARGET_EXTENSION_LIST})
            set(SHADER_OUTPUT_PATH "${SHADERS_OUTPUT_DIR}/${SHADER_RELATIVE_PATH}/${SHADER_NAME}.${SHADER_EXTENSION}")
            add_custom_command(
                OUTPUT ${SHADER_OUTPUT_PATH}
                COMMAND ${SLANG_TOOL} ${SHADER_ABSOLUTE_PATH} -I${CMAKE_CURRENT_SOURCE_DIR}/include/shaders -entry main -profile ${SHADERS_TARGET_PROFILE} -o ${SHADER_OUTPUT_PATH}
                DEPENDS ${SHADER_ABSOLUTE_PATH})
            list(APPEND SHADER_BINARY_FILES ${SHADER_OUTPUT_PATH})
        endforeach(SHADER_EXTENSION ${SHADER_TARGET_EXTENSION_LIST})
    endforeach()

    add_custom_target(${SHADERS_TARGET_NAME} ALL
        DEPENDS ${SHADERS_OUTPUT_DIR} ${SHADER_BINARY_FILES} ${SHADER_META_FILES}
    )
endfunction(shaders_target)

shaders_target(
    NAME vs_shaders
    DIR vs
    PROFILE vs_6_0
)

shaders_target(
    NAME fs_shaders
    DIR fs
    PROFILE ps_6_0
)

storm_headers(
    NAME shader_headers
    HEADERS_DEPS
        hlslpp
    ADDITIONAL_INCLUDE_DIRS
        include
)

add_custom_target(shaders ALL
    DEPENDS vs_shaders fs_shaders shader_headers
)
