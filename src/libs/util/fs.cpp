#include "fs.h"

#include <SDL3/SDL.h>

std::filesystem::path fs::GetStashPath()
{
    static std::filesystem::path path = {};
    if (path.empty()) {
        char* const pref_path = SDL_GetPrefPath("Akella", "Sea Dogs");
        path                  = pref_path;
        SDL_free(pref_path);
    }
    return path;
}

std::filesystem::path fs::GetLogsPath()
{
    return GetStashPath() / "Logs";
}

std::filesystem::path fs::GetSaveDataPath()
{
    return GetStashPath() / "SaveData";
}

std::filesystem::path fs::GetScreenshotsPath()
{
    return GetStashPath() / "Screenshots";
}
