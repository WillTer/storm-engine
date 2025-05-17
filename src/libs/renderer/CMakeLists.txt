if (WIN32)
set(SYSTEM_DEPS "legacy_stdio_definitions")
else()
set(SYSTEM_DEPS "${NATIVE_D3D9_LIBS}")
endif()

STORM_SETUP(
    TARGET_NAME renderer
    TYPE storm_module
    DEPENDENCIES core ${SDK_D3D9_LIBS} util ${SYSTEM_DEPS} fmt
)
