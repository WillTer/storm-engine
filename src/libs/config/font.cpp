#include "font.h"

#include <format>

#include <libs/config/i_config_loader.h>
#include <libs/config/ini_file.h>
#include <libs/util/utf8.h>

#include "ini_helpers.h"

namespace
{
constexpr uint32_t USED_CODES = 0x2070;  // end of https://unicode-table.com/en/blocks/general-punctuation/
}

template <>
struct storm::read_to<storm::FontInfo> {
    static FontInfo from_ini(IniFile const& ini, std::string const& section)
    {
        auto info = FontInfo {
            .aspect_height_constant = ini.find_or(section, "AspectHeightConstant", false),
            .texture                = ini.find_or<std::string>(section, "Texture", ""),
            .technique              = ini.find_or<std::string>(section, "Techniques", ""),
            .texture_width          = ini.find_or(section, "Texture_xsize", 1U),
            .texture_height         = ini.find_or(section, "Texture_ysize", 1U),
            .height                 = ini.find_or(section, "Height", 0U),
            .draw_shadow            = ini.contains(section, "Shadow"),
            .shadow_offset_x        = ini.find_or(section, "Shadow_offsetx", 2U),
            .shadow_offset_y        = ini.find_or(section, "Shadow_offsety", 2U),
            .spacebar               = ini.find_or(section, "Spacebar", 8U),
            .symbol_interval        = ini.find_or(section, "Symbol_interval", 0U),
            .pc_scale               = ini.find_or(section, "pcscale", 1.0F),
            .symbols                = {},
        };

        for (uint32_t codepoint = 30; codepoint < USED_CODES; codepoint++) {
            char unicode_sym[5];
            utf8::CodepointToUtf8(unicode_sym, codepoint);

            std::string key = {};
            if (codepoint >= 'a' && codepoint <= 'z') {
                key = std::format("char_{}_", unicode_sym);
            } else if (codepoint == '=') {
                key = "char_equ";
            } else {
                key = std::format("char_{}", unicode_sym);
            }

            if (!ini.contains(section, key)) { continue; }
            auto const value = ini.find<std::string>(section, key);
            auto       rect  = storm::FRect {};

            auto const left_end = value.find_first_of(',');
            if (left_end == std::string::npos) { throw std::runtime_error("invalid font record"); }
            rect.left = (std::stof(value.substr(0, left_end)) + 0.5F) / static_cast<float>(info.texture_width);

            auto const top_end = value.find_first_of(',', left_end + 1);
            if (top_end == std::string::npos) { throw std::runtime_error("invalid font record"); }
            rect.top = (std::stof(value.substr(left_end + 1, top_end)) + 0.5F) / static_cast<float>(info.texture_height);

            auto const width_end = value.find_first_of(',', top_end + 1);
            if (width_end == std::string::npos) { throw std::runtime_error("invalid font record"); }
            rect.right  = rect.left + (std::stof(value.substr(top_end + 1, width_end)) - 1.0F) / static_cast<float>(info.texture_width);
            rect.bottom = rect.top + (std::stof(value.substr(width_end + 1)) - 1.0F) / static_cast<float>(info.texture_height);

            info.symbols.emplace(codepoint, rect);
        }

        return info;
    }
};

namespace storm::font
{

FontInfo info(IConfigLoader& config_loader, std::filesystem::path const& config_file, std::string const& name)
{
    std::string upper_name = name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), [](unsigned char const c) { return std::toupper(c); });

    auto const& config    = config_loader.open_config_cached(config_file);
    auto        font_info = read_to<FontInfo>::from_ini(config, upper_name);

    return font_info;
}

}  // namespace storm::font
