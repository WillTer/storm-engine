#pragma once

#include <filesystem>

namespace storm::fs
{

static inline auto const RESOURCE_DIR_DEFAULT   = std::filesystem::path() / "resource";
static inline auto const PROGRAM_DIR_DEFAULT    = std::filesystem::path() / "program";
static inline auto const INI_DIR_DEFAULT        = RESOURCE_DIR_DEFAULT / "ini";
static inline auto const ALIASES_DIR_DEFAULT    = INI_DIR_DEFAULT / "aliases";
static inline auto const SOUNDS_DIR_DEFAULT     = RESOURCE_DIR_DEFAULT / "sounds";
static inline auto const VIDEOS_DIR_DEFAULT     = RESOURCE_DIR_DEFAULT / "videos";
static inline auto const ANIMATION_DIR_DEFAULT  = RESOURCE_DIR_DEFAULT / "animation";
static inline auto const MODELS_DIR_DEFAULT     = RESOURCE_DIR_DEFAULT / "models";
static inline auto const FOAM_DIR_DEFAULT       = RESOURCE_DIR_DEFAULT / "foam";
static inline auto const TECHNIQUES_DIR_DEFAULT = RESOURCE_DIR_DEFAULT / "techniques";
static inline auto const PARTICLES_DIR_DEFAULT  = RESOURCE_DIR_DEFAULT / "particles";
static inline auto const TEXTURES_DIR_DEFAULT   = RESOURCE_DIR_DEFAULT / "textures";
static inline auto const SEA_DIR_DEFAULT        = RESOURCE_DIR_DEFAULT / "sea";
static inline auto const SHADERS_DIR_DEFAULT    = RESOURCE_DIR_DEFAULT / "shaders";

static inline auto const MAIN_CONFIG_PATH = std::filesystem::path() / "engine.ini";

}  // namespace storm::fs
