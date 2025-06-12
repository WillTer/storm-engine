#include "main_config.h"

#include <libs/core/core.h>
#include <libs/filesystem/default_paths.h>

using namespace storm;

namespace
{

constexpr int DEFAULT_WINDOW_WIDTH  = 1024;
constexpr int DEFAULT_WINDOW_HEIGHT = 768;

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

GeneralInfo main_config::general_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);

    return {
        .use_steam   = config_file->get("", "steam", false),
        .enable_logs = config_file->get("", "logs", false),
    };
}

WindowInfo main_config::window_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);

    return {
        .width               = config_file->get("window", "width", DEFAULT_WINDOW_WIDTH),
        .height              = config_file->get("window", "height", DEFAULT_WINDOW_HEIGHT),
        .preferred_display   = config_file->get("window", "display", 0),
        .full_screen         = config_file->get("window", "full_screen", false),
        .show_borders        = config_file->get("window", "borders", false),
        .run_in_background   = config_file->get("window", "run_in_background", false),
        .sound_in_background = config_file->get("window", "sound_in_background", true),
        .max_fps             = config_file->get("window", "max_fps", static_cast<uint32_t>(0)),
        .font_config         = config_file->get<std::string>("window", "font_config", ""),
        .font_type           = config_file->get<std::string>("window", "font_type", ""),
    };
}

ScriptInfo main_config::script_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);

    return {
        .entry_point       = config_file->get<std::string>("script", "entry_point"),
        .controls          = config_file->get<std::string>("script", "controls", ""),
        .enable_debuginfo  = config_file->get("script", "debuginfo", false),
        .enable_codefiles  = config_file->get("script", "codefiles", false),
        .enable_runtimelog = config_file->get("script", "runtimelog", false),
        .enable_tracefiles = config_file->get("script", "tracefiles", false),
    };
}

CompatibilityInfo main_config::compatibility_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);

    auto version = get_engine_version_from_string(config_file->get<std::string>("compatibility", "target_version", "latest"));
    if (version == storm::ENGINE_VERSION::UNKNOWN) {
        core.Trace("Unknown target version '%s' in engine compatibility settings", version);
        version = storm::ENGINE_VERSION::LATEST;
    }

    return {
        .target_version      = version,
        .use_lowercase_paths = config_file->get("compatibility", "use_lowercase_paths", false),
    };
}

PathsInfo main_config::paths_info(IConfigLoader& config_loader)
{
    auto const config_file = config_loader.open_config_cached(storm::fs::MAIN_CONFIG_PATH);

    return {
        .resource   = config_file->get("paths", "resource", storm::fs::RESOURCE_DIR_DEFAULT.string()),
        .program    = config_file->get("paths", "program", storm::fs::PROGRAM_DIR_DEFAULT.string()),
        .config     = config_file->get("paths", "config", storm::fs::CONFIG_DIR_DEFAULT.string()),
        .aliases    = config_file->get("paths", "aliases", storm::fs::ALIASES_DIR_DEFAULT.string()),
        .sounds     = config_file->get("paths", "sounds", storm::fs::SOUNDS_DIR_DEFAULT.string()),
        .videos     = config_file->get("paths", "videos", storm::fs::VIDEOS_DIR_DEFAULT.string()),
        .animation  = config_file->get("paths", "animation", storm::fs::ANIMATION_DIR_DEFAULT.string()),
        .models     = config_file->get("paths", "models", storm::fs::MODELS_DIR_DEFAULT.string()),
        .foam       = config_file->get("paths", "foam", storm::fs::FOAM_DIR_DEFAULT.string()),
        .techniques = config_file->get("paths", "techniques", storm::fs::TECHNIQUES_DIR_DEFAULT.string()),
        .particles  = config_file->get("paths", "particles", storm::fs::PARTICLES_DIR_DEFAULT.string()),
        .textures   = config_file->get("paths", "textures", storm::fs::TEXTURES_DIR_DEFAULT.string()),
        .sea        = config_file->get("paths", "sea", storm::fs::SEA_DIR_DEFAULT.string()),
    };
}
