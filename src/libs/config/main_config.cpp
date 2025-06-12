#include "main_config.h"

#include <libs/core/core.h>
#include <libs/filesystem/default_paths.h>
#include <toml.hpp>

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
    .max_fps             = 0,
    .font_config         = "",
    .font_type           = "",
};

ScriptInfo const DEFAULT_SCRIPT_INFO = {
    .entry_point       = "",
    .controls          = "",
    .enable_debuginfo  = false,
    .enable_codefiles  = false,
    .enable_runtimelog = false,
    .enable_tracefiles = false,
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
            .max_fps             = toml::find_or(v, "max_fps", DEFAULT_WINDOW_INFO.max_fps),
            .font_config         = toml::find_or(v, "font_config", DEFAULT_WINDOW_INFO.font_config),
            .font_type           = toml::find_or(v, "font_type", DEFAULT_WINDOW_INFO.font_type),
        };
    }
};

template <>
struct from<storm::ScriptInfo> {
    static storm::ScriptInfo from_toml(toml::value const& v)
    {
        if (!v.is_table()) { return DEFAULT_SCRIPT_INFO; }

        return {
            .entry_point       = toml::find<std::string>(v, "entry_point"),
            .controls          = toml::find_or(v, "controls", DEFAULT_SCRIPT_INFO.controls),
            .enable_debuginfo  = toml::find_or(v, "debuginfo", DEFAULT_SCRIPT_INFO.enable_debuginfo),
            .enable_codefiles  = toml::find_or(v, "codefiles", DEFAULT_SCRIPT_INFO.enable_codefiles),
            .enable_runtimelog = toml::find_or(v, "runtimelog", DEFAULT_SCRIPT_INFO.enable_runtimelog),
            .enable_tracefiles = toml::find_or(v, "tracefiles", DEFAULT_SCRIPT_INFO.enable_tracefiles),
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

}  // namespace toml

GeneralInfo main_config::general_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::get<GeneralInfo>(config_file);
}

WindowInfo main_config::window_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "window", DEFAULT_WINDOW_INFO);
}

ScriptInfo main_config::script_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "script", DEFAULT_SCRIPT_INFO);
}

CompatibilityInfo main_config::compatibility_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "compatibility", DEFAULT_COMPATIBILITY_INFO);
}

PathsInfo main_config::paths_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);
    return toml::find_or(config_file, "paths", DEFAULT_PATHS_INFO);
}
