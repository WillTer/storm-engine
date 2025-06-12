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
    bool        vsync;
    uint32_t    max_fps;
    std::string font_config;
    std::string font_type;
};

struct DeviceInfo {
    int         adapter;
    int         msaa_level;
    float       fov_multiplier;
    float       near_clip_plane;
    float       far_clip_plane;
    bool        post_process;
    std::string screen_bpp;
    std::string screenshot_ext;
    bool        show_exinfo;
    bool        lockable_back_buffer;
    bool        use_large_back_buffer;
    int         texture_degradation_level;
    bool        show_fps;
    bool        safe_rendering;
    bool        texture_log;
    bool        geometry_log;
    bool        drop_video_conveyor;
};

struct ScriptInfo {
    std::string entry_point;
    std::string controls;
    bool        compilation_logs;
    bool        create_codefiles;
    bool        runtime_logs;
    bool        break_on_error;
    int         cache_mode;
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

struct ProgressImageInfo {
    float relative_x;
    float relative_y;
    float relative_width;
    float relative_height;
    int   h_frames_count;
    int   v_frames_count;
};

namespace main_config
{

GeneralInfo       general_info(IConfigLoader& config_loader);
WindowInfo        window_info(IConfigLoader& config_loader);
DeviceInfo        device_info(IConfigLoader& config_loader);
ScriptInfo        script_info(IConfigLoader& config_loader);
CompatibilityInfo compatibility_info(IConfigLoader& config_loader);
PathsInfo         paths_info(IConfigLoader& config_loader);
ProgressImageInfo progress_image_info(IConfigLoader& config_loader);

}  // namespace main_config

}  // namespace storm
