#pragma once

#include <filesystem>
#include <string>

#include <libs/core/engine_version.hpp>

namespace storm
{

class IConfigLoader;

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
    std::string backend;
    int         adapter;
    int         msaa_level;
    float       fov_multiplier;
    float       near_clip_plane;
    float       far_clip_plane;
    bool        post_process;
    std::string screenshot_ext;
    bool        show_fps;
    bool        safe_rendering;
    bool        texture_log;
    bool        geometry_log;
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
    std::filesystem::path ini;
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
    std::filesystem::path shaders;
};

struct ProgressImageInfo {
    bool     frame;
    float    relative_x;
    float    relative_y;
    float    relative_width;
    float    relative_height;
    uint32_t h_frames_count;
    uint32_t v_frames_count;

    float       aspect_ratio;
    std::string progress_texture;
    std::string border_texture;
};

namespace main_config
{

GeneralInfo       general_info(IConfigLoader& config_loader);
WindowInfo        window_info(IConfigLoader& config_loader);
DeviceInfo        device_info(IConfigLoader& config_loader);
SoundInfo         sound_info(IConfigLoader& config_loader);
SeaInfo           sea_info(IConfigLoader& config_loader);
ControlsInfo      controls_info(IConfigLoader& config_loader);
ScriptInfo        script_info(IConfigLoader& config_loader);
CompatibilityInfo compatibility_info(IConfigLoader& config_loader);
PathsInfo         paths_info(IConfigLoader& config_loader);
ProgressImageInfo progress_image_info(IConfigLoader& config_loader);

}  // namespace main_config

}  // namespace storm
