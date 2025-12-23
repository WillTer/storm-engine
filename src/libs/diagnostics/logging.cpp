#include "logging.hpp"

#include <SDL3/SDL_log.h>
#include <libs/util/fs.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace
{
constexpr auto LOG_EXTENSION = "log";

constexpr std::string_view get_sdl_category_name(int const category)
{
    switch (category) {
#define CATEGORY(category) \
    case SDL_LOG_CATEGORY_##category: return "[" #category "] "; break
        CATEGORY(APPLICATION);
        CATEGORY(ERROR);
        CATEGORY(ASSERT);
        CATEGORY(SYSTEM);
        CATEGORY(AUDIO);
        CATEGORY(VIDEO);
        CATEGORY(RENDER);
        CATEGORY(INPUT);
        CATEGORY(TEST);
        CATEGORY(GPU);
#undef CATEGORY
    default: break;
    }

    return {};
}

constexpr spdlog::level::level_enum get_level_from_priority(SDL_LogPriority const priority)
{
    switch (priority) {
#define PRIORITY(priority) \
    case SDL_LOG_PRIORITY_##priority: return static_cast<spdlog::level::level_enum>(SPDLOG_LEVEL_##priority); break
    case SDL_LOG_PRIORITY_VERBOSE:
        [[fallthrough]];
        PRIORITY(TRACE);
        PRIORITY(DEBUG);
        PRIORITY(INFO);
        PRIORITY(WARN);
        PRIORITY(ERROR);
        PRIORITY(CRITICAL);
#undef PRIORITY
    default: break;
    }

    return spdlog::level::off;
}

constexpr SDL_LogPriority get_priority_from_level(spdlog::level::level_enum const level)
{
    switch (static_cast<int>(level)) {
#define PRIORITY(priority) \
    case SPDLOG_LEVEL_##priority: return SDL_LOG_PRIORITY_##priority; break
        PRIORITY(TRACE);
        PRIORITY(DEBUG);
        PRIORITY(INFO);
        PRIORITY(WARN);
        PRIORITY(ERROR);
        PRIORITY(CRITICAL);
#undef PRIORITY
    default: break;
    }

    return SDL_LOG_PRIORITY_INVALID;
}

void SDLCALL log_output(void* userdata, int category, SDL_LogPriority priority, char const* message)
{
    auto* logger = static_cast<spdlog::logger*>(userdata);
    logger->log(get_level_from_priority(priority), "{} {}", get_sdl_category_name(category), message);
}

}  // namespace

namespace storm::logging
{

void init_logger_for_sdl(spdlog::level::level_enum const level)
{
    auto const logger = get_logger("sdl");
    SDL_SetLogOutputFunction(log_output, logger.get());
    SDL_SetLogPriorities(get_priority_from_level(level));
}

logger_ptr get_logger_with_stdout(std::string const& name, spdlog::level::level_enum const level, bool truncate)
{
    auto logger = spdlog::get(name);
    if (logger) { return logger; }

    auto path = fs::GetLogsPath() / name;
    path.replace_extension(LOG_EXTENSION);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(level);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), truncate);
    console_sink->set_level(level);

    logger = std::make_shared<spdlog::logger>(name, spdlog::sinks_init_list {console_sink, file_sink});
    spdlog::register_logger(logger);
    logger->set_level(level);

    return logger;
}

logger_ptr get_logger(std::string const& name, spdlog::level::level_enum const level, bool const truncate)
{
    auto logger = spdlog::get(name);
    if (logger) { return logger; }

    auto path = fs::GetLogsPath() / name;
    path.replace_extension(LOG_EXTENSION);

    logger = spdlog::basic_logger_mt(name, path.string(), truncate);
    logger->set_level(level);

    return logger;
}

}  // namespace storm::logging
