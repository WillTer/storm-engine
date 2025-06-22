#include "main_config.h"

#include <algorithm>

#include <libs/core/core.h>
#include <libs/filesystem/default_paths.h>

#include "i_config_loader.h"
#include "ini_file.h"
#include "ini_helpers.h"

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

constexpr SoundInfo DEFAULT_SOUND_INFO = {
    .fade_time_ms = 500,
};

constexpr SeaInfo DEFAULT_SEA_INFO = {
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

constexpr CompatibilityInfo DEFAULT_COMPATIBILITY_INFO = {
    .target_version      = ENGINE_VERSION::LATEST,
    .use_lowercase_paths = false,
};

PathsInfo const DEFAULT_PATHS_INFO = {
    .resource   = fs::RESOURCE_DIR_DEFAULT,
    .program    = fs::PROGRAM_DIR_DEFAULT,
    .ini        = fs::INI_DIR_DEFAULT,
    .aliases    = fs::ALIASES_DIR_DEFAULT,
    .sounds     = fs::SOUNDS_DIR_DEFAULT,
    .videos     = fs::VIDEOS_DIR_DEFAULT,
    .animation  = fs::ANIMATION_DIR_DEFAULT,
    .models     = fs::MODELS_DIR_DEFAULT,
    .foam       = fs::FOAM_DIR_DEFAULT,
    .techniques = fs::TECHNIQUES_DIR_DEFAULT,
    .particles  = fs::PARTICLES_DIR_DEFAULT,
    .textures   = fs::TEXTURES_DIR_DEFAULT,
    .sea        = fs::SEA_DIR_DEFAULT,
    .shaders    = fs::SHADERS_DIR_DEFAULT,
};

constexpr ProgressImageInfo DEFAULT_PROGRESS_IMAGE_INFO = {
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

template <>
struct storm::read_to<GeneralInfo> {
    static GeneralInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .use_steam   = ini.find_or(section, "steam", DEFAULT_GENERAL_INFO.use_steam),
            .enable_logs = ini.find_or(section, "logs", DEFAULT_GENERAL_INFO.enable_logs),
        };
    }
};

template <>
struct storm::read_to<WindowInfo> {
    static WindowInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .width               = ini.find_or(section, "width", DEFAULT_WINDOW_INFO.width),
            .height              = ini.find_or(section, "height", DEFAULT_WINDOW_INFO.height),
            .preferred_display   = ini.find_or(section, "preferred_display", DEFAULT_WINDOW_INFO.preferred_display),
            .full_screen         = ini.find_or(section, "full_screen", DEFAULT_WINDOW_INFO.full_screen),
            .show_borders        = ini.find_or(section, "show_borders", DEFAULT_WINDOW_INFO.show_borders),
            .run_in_background   = ini.find_or(section, "run_in_background", DEFAULT_WINDOW_INFO.run_in_background),
            .sound_in_background = ini.find_or(section, "sound_in_background", DEFAULT_WINDOW_INFO.sound_in_background),
            .vsync               = ini.find_or(section, "vsync", DEFAULT_WINDOW_INFO.vsync),
            .max_fps             = ini.find_or(section, "max_fps", DEFAULT_WINDOW_INFO.max_fps),
            .font_config         = ini.find_or(section, "font_config", DEFAULT_WINDOW_INFO.font_config),
            .font_type           = ini.find_or(section, "font_type", DEFAULT_WINDOW_INFO.font_type),
        };
    }
};

template <>
struct storm::read_to<DeviceInfo> {
    static DeviceInfo from_ini(IniFile const& ini, std::string const& section)
    {
        auto screenshot_ext = ini.find_or(section, "screenshot_ext", DEFAULT_DEVICE_INFO.screenshot_ext);
        std::transform(
            screenshot_ext.begin(), screenshot_ext.end(), screenshot_ext.begin(), [](unsigned char const c) { return std::tolower(c); });

        return {
            .adapter                   = ini.find_or(section, "adapter", DEFAULT_DEVICE_INFO.adapter),
            .msaa_level                = ini.find_or(section, "msaa_level", DEFAULT_DEVICE_INFO.msaa_level),
            .fov_multiplier            = ini.find_or(section, "fov_multiplier", DEFAULT_DEVICE_INFO.fov_multiplier),
            .near_clip_plane           = ini.find_or(section, "near_clip_plane", DEFAULT_DEVICE_INFO.near_clip_plane),
            .far_clip_plane            = ini.find_or(section, "far_clip_plane", DEFAULT_DEVICE_INFO.far_clip_plane),
            .post_process              = ini.find_or(section, "post_process", DEFAULT_DEVICE_INFO.post_process),
            .screen_bpp                = ini.find_or(section, "screen_bpp", DEFAULT_DEVICE_INFO.screen_bpp),
            .screenshot_ext            = ini.find_or(section, "screenshot_ext", DEFAULT_DEVICE_INFO.screenshot_ext),
            .show_exinfo               = ini.find_or(section, "show_exinfo", DEFAULT_DEVICE_INFO.show_exinfo),
            .lockable_back_buffer      = ini.find_or(section, "lockable_back_buffer", DEFAULT_DEVICE_INFO.lockable_back_buffer),
            .use_large_back_buffer     = ini.find_or(section, "use_large_back_buffer", DEFAULT_DEVICE_INFO.use_large_back_buffer),
            .texture_degradation_level = ini.find_or(section, "texture_degradation_level", DEFAULT_DEVICE_INFO.texture_degradation_level),
            .show_fps                  = ini.find_or(section, "show_fps", DEFAULT_DEVICE_INFO.show_fps),
            .safe_rendering            = ini.find_or(section, "safe_rendering", DEFAULT_DEVICE_INFO.safe_rendering),
            .texture_log               = ini.find_or(section, "texture_log", DEFAULT_DEVICE_INFO.texture_log),
            .geometry_log              = ini.find_or(section, "geometry_log", DEFAULT_DEVICE_INFO.geometry_log),
            .drop_video_conveyor       = ini.find_or(section, "drop_video_conveyor", DEFAULT_DEVICE_INFO.drop_video_conveyor),
        };
    }
};

template <>
struct storm::read_to<SoundInfo> {
    static SoundInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .fade_time_ms = ini.find_or(section, "fade_time_ms", DEFAULT_SOUND_INFO.fade_time_ms),
        };
    }
};

template <>
struct storm::read_to<SeaInfo> {
    static SeaInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .enable_foam = ini.find_or(section, "enable_foam", DEFAULT_SEA_INFO.enable_foam),
        };
    }
};

template <>
struct storm::read_to<ControlsInfo> {
    static ControlsInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .scheme         = ini.find_or(section, "scheme", DEFAULT_CONTROLS_INFO.scheme),
            .use_debug_keys = ini.find_or(section, "use_debug_keys", DEFAULT_CONTROLS_INFO.use_debug_keys),
        };
    }
};

template <>
struct storm::read_to<ScriptInfo> {
    static ScriptInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .entry_point      = ini.find<std::string>(section, "entry_point"),
            .compilation_logs = ini.find_or(section, "compilation_logs", DEFAULT_SCRIPT_INFO.compilation_logs),
            .create_codefiles = ini.find_or(section, "create_codefiles", DEFAULT_SCRIPT_INFO.create_codefiles),
            .runtime_logs     = ini.find_or(section, "runtime_logs", DEFAULT_SCRIPT_INFO.runtime_logs),
            .break_on_error   = ini.find_or(section, "break_on_error", DEFAULT_SCRIPT_INFO.break_on_error),
            .cache_mode       = get_cache_mode_from_string(ini.find_or<std::string>(section, "cache_mode", "disabled")),
        };
    }
};

template <>
struct storm::read_to<CompatibilityInfo> {
    static CompatibilityInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .target_version      = get_engine_version_from_string(ini.find_or<std::string>(section, "target_version", "latest")),
            .use_lowercase_paths = ini.find_or(section, "use_lowercase_paths", DEFAULT_COMPATIBILITY_INFO.use_lowercase_paths),
        };
    }
};

template <>
struct storm::read_to<PathsInfo> {
    static PathsInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .resource   = ini.find_or(section, "resource", DEFAULT_PATHS_INFO.resource.string()),
            .program    = ini.find_or(section, "program", DEFAULT_PATHS_INFO.program.string()),
            .ini        = ini.find_or(section, "ini", DEFAULT_PATHS_INFO.ini.string()),
            .aliases    = ini.find_or(section, "aliases", DEFAULT_PATHS_INFO.aliases.string()),
            .sounds     = ini.find_or(section, "sounds", DEFAULT_PATHS_INFO.sounds.string()),
            .videos     = ini.find_or(section, "videos", DEFAULT_PATHS_INFO.videos.string()),
            .animation  = ini.find_or(section, "animation", DEFAULT_PATHS_INFO.animation.string()),
            .models     = ini.find_or(section, "models", DEFAULT_PATHS_INFO.models.string()),
            .foam       = ini.find_or(section, "foam", DEFAULT_PATHS_INFO.foam.string()),
            .techniques = ini.find_or(section, "techniques", DEFAULT_PATHS_INFO.techniques.string()),
            .particles  = ini.find_or(section, "particles", DEFAULT_PATHS_INFO.particles.string()),
            .textures   = ini.find_or(section, "textures", DEFAULT_PATHS_INFO.textures.string()),
            .sea        = ini.find_or(section, "sea", DEFAULT_PATHS_INFO.sea.string()),
            .shaders    = ini.find_or(section, "shaders", DEFAULT_PATHS_INFO.shaders.string()),
        };
    }
};

template <>
struct storm::read_to<ProgressImageInfo> {
    static ProgressImageInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .frame           = ini.find_or(section, "frame", DEFAULT_PROGRESS_IMAGE_INFO.frame),
            .relative_x      = ini.find_or(section, "relative_x", DEFAULT_PROGRESS_IMAGE_INFO.relative_x),
            .relative_y      = ini.find_or(section, "relative_y", DEFAULT_PROGRESS_IMAGE_INFO.relative_y),
            .relative_width  = ini.find_or(section, "relative_width", DEFAULT_PROGRESS_IMAGE_INFO.relative_width),
            .relative_height = ini.find_or(section, "relative_height", DEFAULT_PROGRESS_IMAGE_INFO.relative_height),
            .h_frames_count  = ini.find_or(section, "h_frames_count", DEFAULT_PROGRESS_IMAGE_INFO.h_frames_count),
            .v_frames_count  = ini.find_or(section, "v_frames_count", DEFAULT_PROGRESS_IMAGE_INFO.v_frames_count),
        };
    }
};

GeneralInfo main_config::general_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<GeneralInfo>::from_ini(config_file, "");
}

WindowInfo main_config::window_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<WindowInfo>::from_ini(config_file, "window");
}

DeviceInfo main_config::device_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<DeviceInfo>::from_ini(config_file, "device");
}

SoundInfo main_config::sound_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<SoundInfo>::from_ini(config_file, "sound");
}

SeaInfo main_config::sea_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<SeaInfo>::from_ini(config_file, "sea");
}

ControlsInfo main_config::controls_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<ControlsInfo>::from_ini(config_file, "controls");
}

ScriptInfo main_config::script_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<ScriptInfo>::from_ini(config_file, "script");
}

CompatibilityInfo main_config::compatibility_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<CompatibilityInfo>::from_ini(config_file, "compatibility");
}

PathsInfo main_config::paths_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<PathsInfo>::from_ini(config_file, "paths");
}

ProgressImageInfo main_config::progress_image_info()
{
    auto const& config_file = config_loader->open_config_cached(fs::MAIN_CONFIG_PATH);
    return read_to<ProgressImageInfo>::from_ini(config_file, "progress_image_info");
}
