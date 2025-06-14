#pragma once

#include <filesystem>
#include <string>

#include <libs/core/engine_version.hpp>

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

struct SoundInfo {
    uint64_t fade_time_ms;
};

struct SeaInfo {
    bool enable_foam;
};

struct ControlsInfo {
    std::string scheme;
    bool        use_debug_keys;
};

struct ScriptInfo {
    std::string entry_point;
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
    int   frame;
    float relative_x;
    float relative_y;
    float relative_width;
    float relative_height;
    int   h_frames_count;
    int   v_frames_count;
};

namespace main_config
{

GeneralInfo       general_info();
WindowInfo        window_info();
DeviceInfo        device_info();
SoundInfo         sound_info();
SeaInfo           sea_info();
ControlsInfo      controls_info();
ScriptInfo        script_info();
CompatibilityInfo compatibility_info();
PathsInfo         paths_info();
ProgressImageInfo progress_image_info();

}  // namespace main_config

}  // namespace storm
