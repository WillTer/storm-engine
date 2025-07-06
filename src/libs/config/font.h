#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

#include <libs/renderer_next/types.h>

namespace storm
{

class IConfigLoader;

struct FontInfo {
    bool        aspect_height_constant;
    std::string texture;
    std::string technique;
    uint32_t    texture_width;
    uint32_t    texture_height;
    uint32_t    height;
    bool        draw_shadow;
    uint32_t    shadow_offset_x;
    uint32_t    shadow_offset_y;
    uint32_t    spacebar;
    uint32_t    symbol_interval;
    float       pc_scale;

    std::unordered_map<uint32_t, storm::FRect> symbols;
};

namespace font
{

FontInfo info(IConfigLoader& config_loader, std::filesystem::path const& config_file, std::string const& name);

}

}  // namespace storm
