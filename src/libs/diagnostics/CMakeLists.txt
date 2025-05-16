STORM_SETUP(
    TARGET_NAME diagnostics
    TYPE library
    DEPENDENCIES core util spdlog::spdlog sentry::sentry
    DEFINES spdlog_EXPORTS
)
