#pragma once
#include <filesystem>
#include <string>

#include <spdlog/logger.h>

namespace storm::logging
{

using logger_ptr = std::shared_ptr<spdlog::logger>;

void init_logger_for_sdl(spdlog::level::level_enum level = spdlog::level::trace);

logger_ptr get_logger_with_stdout(std::string const& name, spdlog::level::level_enum level = spdlog::level::trace, bool truncate = true);
logger_ptr get_logger(std::string const& name, spdlog::level::level_enum level = spdlog::level::trace, bool truncate = true);

}  // namespace storm::logging
