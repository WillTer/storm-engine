#pragma once

#include <filesystem>

static inline auto const RESOURCE_DIR             = std::filesystem::path("resource");
static inline auto const RESOURCE_INI_DIR         = RESOURCE_DIR / "ini";
static inline auto const RESOURCE_INI_ALIASES_DIR = RESOURCE_INI_DIR / "aliases";
static inline auto const RESOURCE_SOUNDS_DIR      = RESOURCE_DIR / "sounds";
static inline auto const RESOURCE_VIDEOS_DIR      = RESOURCE_DIR / "videos";
static inline auto const RESOURCE_ANIMATION_DIR   = RESOURCE_DIR / "animation";
static inline auto const RESOURCE_MODELS_DIR      = RESOURCE_DIR / "models";
static inline auto const RESOURCE_FOAM_DIR        = RESOURCE_DIR / "foam";
static inline auto const RESOURCE_TECHNIQUES_DIR  = RESOURCE_DIR / "techniques";
static inline auto const RESOURCE_PARTICLES_DIR   = RESOURCE_DIR / "particles";
static inline auto const RESOURCE_TEXTURES_DIR    = RESOURCE_DIR / "textures";
static inline auto const RESOURCE_SEA_DIR         = RESOURCE_DIR / "sea";
