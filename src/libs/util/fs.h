#pragma once

#include <filesystem>

/* Filesystem proxy */
namespace fs
{

std::filesystem::path GetStashPath();
std::filesystem::path GetLogsPath();
std::filesystem::path GetSaveDataPath();
std::filesystem::path GetScreenshotsPath();

}  // namespace fs
