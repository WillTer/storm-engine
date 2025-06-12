#pragma once

#include <string>

#include <libs/core/engine_version.hpp>

#include "i_config_loader.h"

namespace storm
{

struct GeneralInfo {
    bool use_steam;
    bool enable_logs;
};

struct WindowInfo {
    int         width;
    int         height;
    int         preferred_display;
    bool        full_screen;
    bool        show_borders;
    bool        run_in_background;
    bool        sound_in_background;
    uint32_t    max_fps;
    std::string font_config;
    std::string font_type;
};

struct ScriptInfo {
    std::string entry_point;
    std::string controls;
    bool        enable_debuginfo;
    bool        enable_codefiles;
    bool        enable_runtimelog;
    bool        enable_tracefiles;
};

struct CompatibilityInfo {
    ENGINE_VERSION target_version;
    bool           use_lowercase_paths;
};

struct PathsInfo {
    std::filesystem::path resource;
    std::filesystem::path program;
    std::filesystem::path config;
    std::filesystem::path aliases;
    std::filesystem::path sounds;
    std::filesystem::path videos;
    std::filesystem::path animation;
    std::filesystem::path models;
    std::filesystem::path foam;
    std::filesystem::path techniques;
    std::filesystem::path particles;
    std::filesystem::path textures;
    std::filesystem::path sea;
};

namespace main_config
{

GeneralInfo       general_info(IConfigLoader& config_loader);
WindowInfo        window_info(IConfigLoader& config_loader);
ScriptInfo        script_info(IConfigLoader& config_loader);
CompatibilityInfo compatibility_info(IConfigLoader& config_loader);
PathsInfo         paths_info(IConfigLoader& config_loader);

}  // namespace main_config

}  // namespace storm
