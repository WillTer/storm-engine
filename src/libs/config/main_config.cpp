#include "main_config.h"

#include <algorithm>

#include <libs/core/core.h>
#include <libs/filesystem/default_paths.h>
#include <toml.hpp>

#include "i_config_loader.h"

using namespace storm;

namespace
{

constexpr GeneralInfo DEFAULT_GENERAL_INFO = {
    .use_steam   = false,
    .enable_logs = true,
};

WindowInfo const DEFAULT_WINDOW_INFO = {
    .width               = 1024,
    .height              = 768,
    .preferred_display   = 0,
    .full_screen         = false,
    .show_borders        = false,
    .run_in_background   = false,
    .sound_in_background = true,
    .vsync               = false,
    .max_fps             = 0,
    .font_config         = "",
    .font_type           = "",
};

DeviceInfo const DEFAULT_DEVICE_INFO = {
    .adapter                   = std::numeric_limits<int32_t>::max(),
    .msaa_level                = 0,
    .fov_multiplier            = 1.0F,
    .near_clip_plane           = 0.1F,
    .far_clip_plane            = 4000.0F,
    .post_process              = false,
    .screen_bpp                = "",
    .screenshot_ext            = "jpg",
    .show_exinfo               = false,
    .lockable_back_buffer      = false,
    .use_large_back_buffer     = false,
    .texture_degradation_level = 0,
    .show_fps                  = false,
    .safe_rendering            = false,
    .texture_log               = false,
    .geometry_log              = false,
    .drop_video_conveyor       = false,
};

SoundInfo const DEFAULT_SOUND_INFO = {
    .fade_time_ms = 500,
};

SeaInfo const DEFAULT_SEA_INFO = {
    .enable_foam = true,
};

ControlsInfo const DEFAULT_CONTROLS_INFO = {
    .scheme         = "",
    .use_debug_keys = false,
};

ScriptInfo const DEFAULT_SCRIPT_INFO = {
    .entry_point      = "",
    .compilation_logs = false,
    .create_codefiles = false,
    .runtime_logs     = false,
    .break_on_error   = false,
    .cache_mode       = 0,
};

CompatibilityInfo const DEFAULT_COMPATIBILITY_INFO = {
    .target_version      = ENGINE_VERSION::LATEST,
    .use_lowercase_paths = false,
};

PathsInfo const DEFAULT_PATHS_INFO = {
    .resource   = storm::fs::RESOURCE_DIR_DEFAULT,
    .program    = storm::fs::PROGRAM_DIR_DEFAULT,
    .config     = storm::fs::CONFIG_DIR_DEFAULT,
    .aliases    = storm::fs::ALIASES_DIR_DEFAULT,
    .sounds     = storm::fs::SOUNDS_DIR_DEFAULT,
    .videos     = storm::fs::VIDEOS_DIR_DEFAULT,
    .animation  = storm::fs::ANIMATION_DIR_DEFAULT,
    .models     = storm::fs::MODELS_DIR_DEFAULT,
    .foam       = storm::fs::FOAM_DIR_DEFAULT,
    .techniques = storm::fs::TECHNIQUES_DIR_DEFAULT,
    .particles  = storm::fs::PARTICLES_DIR_DEFAULT,
    .textures   = storm::fs::TEXTURES_DIR_DEFAULT,
    .sea        = storm::fs::SEA_DIR_DEFAULT,
};

ProgressImageInfo const DEFAULT_PROGRESS_IMAGE_INFO = {
    .frame           = 0,
    .relative_x      = 0.85F,
    .relative_y      = 0.8F,
    .relative_width  = 0.0625F,
    .relative_height = 0.0625F,
    .h_frames_count  = 8,
    .v_frames_count  = 8,
};

ENGINE_VERSION get_engine_version_from_string(std::string const& version)
{
    if (version == "sd") { return ENGINE_VERSION::SEA_DOGS; }
    if (version == "potc") { return ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN; }
    if (version == "ct") { return ENGINE_VERSION::CARIBBEAN_TALES; }
    if (version == "coas") { return ENGINE_VERSION::CITY_OF_ABANDONED_SHIPS; }
    if (version == "teho") { return ENGINE_VERSION::TO_EACH_HIS_OWN; }
    if (version == "latest") { return ENGINE_VERSION::LATEST; }

    return ENGINE_VERSION::UNKNOWN;
}

int get_cache_mode_from_string(std::string const& mode)
{
    if (mode == "enabled") { return 1; }
    if (mode == "no_runtime_check") { return 2; }

    return 0;
}

}  // namespace

namespace toml
{

template <>
struct from<storm::GeneralInfo> {
    static storm::GeneralInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_GENERAL_INFO; }

        return {
            .use_steam   = toml::find_or(v, "steam", DEFAULT_GENERAL_INFO.use_steam),
            .enable_logs = toml::find_or(v, "logs", DEFAULT_GENERAL_INFO.enable_logs),
        };
    }
};

template <>
struct from<storm::WindowInfo> {
    static storm::WindowInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_WINDOW_INFO; }

        return {
            .width               = toml::find_or(v, "width", DEFAULT_WINDOW_INFO.width),
            .height              = toml::find_or(v, "height", DEFAULT_WINDOW_INFO.height),
            .preferred_display   = toml::find_or(v, "preferred_display", DEFAULT_WINDOW_INFO.preferred_display),
            .full_screen         = toml::find_or(v, "full_screen", DEFAULT_WINDOW_INFO.full_screen),
            .show_borders        = toml::find_or(v, "show_borders", DEFAULT_WINDOW_INFO.show_borders),
            .run_in_background   = toml::find_or(v, "run_in_background", DEFAULT_WINDOW_INFO.run_in_background),
            .sound_in_background = toml::find_or(v, "sound_in_background", DEFAULT_WINDOW_INFO.sound_in_background),
            .vsync               = toml::find_or(v, "vsync", DEFAULT_WINDOW_INFO.vsync),
            .max_fps             = toml::find_or(v, "max_fps", DEFAULT_WINDOW_INFO.max_fps),
            .font_config         = toml::find_or(v, "font_config", DEFAULT_WINDOW_INFO.font_config),
            .font_type           = toml::find_or(v, "font_type", DEFAULT_WINDOW_INFO.font_type),
        };
    }
};

template <>
struct from<storm::DeviceInfo> {
    static storm::DeviceInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_DEVICE_INFO; }

        auto screenshot_ext = toml::find_or(v, "screenshot_ext", DEFAULT_DEVICE_INFO.screenshot_ext);
        std::transform(
            screenshot_ext.begin(), screenshot_ext.end(), screenshot_ext.begin(), [](unsigned char const c) { return std::tolower(c); });

        return {
            .adapter                   = toml::find_or(v, "adapter", DEFAULT_DEVICE_INFO.adapter),
            .msaa_level                = toml::find_or(v, "msaa_level", DEFAULT_DEVICE_INFO.msaa_level),
            .fov_multiplier            = toml::find_or(v, "fov_multiplier", DEFAULT_DEVICE_INFO.fov_multiplier),
            .near_clip_plane           = toml::find_or(v, "near_clip_plane", DEFAULT_DEVICE_INFO.near_clip_plane),
            .far_clip_plane            = toml::find_or(v, "far_clip_plane", DEFAULT_DEVICE_INFO.far_clip_plane),
            .post_process              = toml::find_or(v, "post_process", DEFAULT_DEVICE_INFO.post_process),
            .screen_bpp                = toml::find_or(v, "screen_bpp", DEFAULT_DEVICE_INFO.screen_bpp),
            .screenshot_ext            = toml::find_or(v, "screenshot_ext", DEFAULT_DEVICE_INFO.screenshot_ext),
            .show_exinfo               = toml::find_or(v, "show_exinfo", DEFAULT_DEVICE_INFO.show_exinfo),
            .lockable_back_buffer      = toml::find_or(v, "lockable_back_buffer", DEFAULT_DEVICE_INFO.lockable_back_buffer),
            .use_large_back_buffer     = toml::find_or(v, "use_large_back_buffer", DEFAULT_DEVICE_INFO.use_large_back_buffer),
            .texture_degradation_level = toml::find_or(v, "texture_degradation_level", DEFAULT_DEVICE_INFO.texture_degradation_level),
            .show_fps                  = toml::find_or(v, "show_fps", DEFAULT_DEVICE_INFO.show_fps),
            .safe_rendering            = toml::find_or(v, "safe_rendering", DEFAULT_DEVICE_INFO.safe_rendering),
            .texture_log               = toml::find_or(v, "texture_log", DEFAULT_DEVICE_INFO.texture_log),
            .geometry_log              = toml::find_or(v, "geometry_log", DEFAULT_DEVICE_INFO.geometry_log),
            .drop_video_conveyor       = toml::find_or(v, "drop_video_conveyor", DEFAULT_DEVICE_INFO.drop_video_conveyor),
        };
    }
};

template <>
struct from<storm::SoundInfo> {
    static storm::SoundInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_SOUND_INFO; }

        return {
            .fade_time_ms = toml::find_or(v, "fade_time_ms", DEFAULT_SOUND_INFO.fade_time_ms),
        };
    }
};

template <>
struct from<storm::SeaInfo> {
    static storm::SeaInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_SEA_INFO; }

        return {
            .enable_foam = toml::find_or(v, "enable_foam", DEFAULT_SEA_INFO.enable_foam),
        };
    }
};

template <>
struct from<storm::ControlsInfo> {
    static storm::ControlsInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_CONTROLS_INFO; }

        return {
            .scheme         = toml::find_or(v, "scheme", DEFAULT_CONTROLS_INFO.scheme),
            .use_debug_keys = toml::find_or(v, "use_debug_keys", DEFAULT_CONTROLS_INFO.use_debug_keys),
        };
    }
};

template <>
struct from<storm::ScriptInfo> {
    static storm::ScriptInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_SCRIPT_INFO; }

        return {
            .entry_point      = toml::find<std::string>(v, "entry_point"),
            .compilation_logs = toml::find_or(v, "compilation_logs", DEFAULT_SCRIPT_INFO.compilation_logs),
            .create_codefiles = toml::find_or(v, "create_codefiles", DEFAULT_SCRIPT_INFO.create_codefiles),
            .runtime_logs     = toml::find_or(v, "runtime_logs", DEFAULT_SCRIPT_INFO.runtime_logs),
            .break_on_error   = toml::find_or(v, "break_on_error", DEFAULT_SCRIPT_INFO.break_on_error),
            .cache_mode       = get_cache_mode_from_string(toml::find_or<std::string>(v, "cache_mode", "disabled")),
        };
    }
};

template <>
struct from<storm::CompatibilityInfo> {
    static storm::CompatibilityInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_COMPATIBILITY_INFO; }

        return {
            .target_version      = get_engine_version_from_string(toml::find_or<std::string>(v, "target_version", "latest")),
            .use_lowercase_paths = toml::find_or(v, "use_lowercase_paths", DEFAULT_COMPATIBILITY_INFO.use_lowercase_paths),
        };
    }
};

template <>
struct from<storm::PathsInfo> {
    static storm::PathsInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_PATHS_INFO; }

        return {
            .resource   = toml::find_or(v, "resource", DEFAULT_PATHS_INFO.resource.string()),
            .program    = toml::find_or(v, "program", DEFAULT_PATHS_INFO.program.string()),
            .config     = toml::find_or(v, "config", DEFAULT_PATHS_INFO.config.string()),
            .aliases    = toml::find_or(v, "aliases", DEFAULT_PATHS_INFO.aliases.string()),
            .sounds     = toml::find_or(v, "sounds", DEFAULT_PATHS_INFO.sounds.string()),
            .videos     = toml::find_or(v, "videos", DEFAULT_PATHS_INFO.videos.string()),
            .animation  = toml::find_or(v, "animation", DEFAULT_PATHS_INFO.animation.string()),
            .models     = toml::find_or(v, "models", DEFAULT_PATHS_INFO.models.string()),
            .foam       = toml::find_or(v, "foam", DEFAULT_PATHS_INFO.foam.string()),
            .techniques = toml::find_or(v, "techniques", DEFAULT_PATHS_INFO.techniques.string()),
            .particles  = toml::find_or(v, "particles", DEFAULT_PATHS_INFO.particles.string()),
            .textures   = toml::find_or(v, "textures", DEFAULT_PATHS_INFO.textures.string()),
            .sea        = toml::find_or(v, "sea", DEFAULT_PATHS_INFO.sea.string()),
        };
    }
};

template <>
struct from<storm::ProgressImageInfo> {
    static storm::ProgressImageInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_PROGRESS_IMAGE_INFO; }

        return {
            .frame           = toml::find_or(v, "frame", DEFAULT_PROGRESS_IMAGE_INFO.frame),
            .relative_x      = toml::find_or(v, "relative_x", DEFAULT_PROGRESS_IMAGE_INFO.relative_x),
            .relative_y      = toml::find_or(v, "relative_y", DEFAULT_PROGRESS_IMAGE_INFO.relative_y),
            .relative_width  = toml::find_or(v, "relative_width", DEFAULT_PROGRESS_IMAGE_INFO.relative_width),
            .relative_height = toml::find_or(v, "relative_height", DEFAULT_PROGRESS_IMAGE_INFO.relative_height),
            .h_frames_count  = toml::find_or(v, "h_frames_count", DEFAULT_PROGRESS_IMAGE_INFO.h_frames_count),
            .v_frames_count  = toml::find_or(v, "v_frames_count", DEFAULT_PROGRESS_IMAGE_INFO.v_frames_count),
        };
    }
};

}  // namespace toml

GeneralInfo main_config::general_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::get<GeneralInfo>(config_file);
}

WindowInfo main_config::window_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "window", DEFAULT_WINDOW_INFO);
}

DeviceInfo main_config::device_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "device", DEFAULT_DEVICE_INFO);
}

SoundInfo main_config::sound_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "sound", DEFAULT_SOUND_INFO);
}

SeaInfo main_config::sea_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "sea", DEFAULT_SEA_INFO);
}

ControlsInfo main_config::controls_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "controls", DEFAULT_CONTROLS_INFO);
}

ScriptInfo main_config::script_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "script", DEFAULT_SCRIPT_INFO);
}

CompatibilityInfo main_config::compatibility_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "compatibility", DEFAULT_COMPATIBILITY_INFO);
}

PathsInfo main_config::paths_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "paths", DEFAULT_PATHS_INFO);
}

ProgressImageInfo main_config::progress_image_info()
{
    auto const config_file = config_loader->open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "progress_image_info", DEFAULT_PROGRESS_IMAGE_INFO);
}
