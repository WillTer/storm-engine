#include "texture_sequence.h"

#include <format>

#include <libs/config/i_config_loader.h>
#include <libs/config/ini_file.h>

#include "ini_helpers.h"

namespace
{
constexpr std::string_view CONFIG_FILE    = "texturesequence.ini";  // TODO: remove hardcode (or not)
constexpr uint32_t         DEFAULT_DELAY  = 128;
constexpr uint32_t         DEFAULT_WIDTH  = 128;
constexpr uint32_t         DEFAULT_HEIGHT = 128;
}  // namespace

template <>
struct storm::read_to<storm::TextureSequenceInfo> {
    static TextureSequenceInfo from_ini(IniFile const& ini, std::string const& section)
    {
        return {
            .texture_file   = ini.find_or<std::string>(section, "textureFile", ""),
            .flip_h         = ini.find_or(section, "flipH", false),
            .flip_v         = ini.find_or(section, "flipV", false),
            .time_delay     = ini.find_or(section, "timeDelay", DEFAULT_DELAY),
            .width          = ini.find_or(section, "width", DEFAULT_WIDTH),
            .height         = ini.find_or(section, "height", DEFAULT_HEIGHT),
            .h_frames_count = ini.find_or(section, "horzQ", 1U),
            .v_frames_count = ini.find_or(section, "vertQ", 1U),
        };
    }
};

namespace storm::texture_sequence
{

TextureSequenceInfo info(IConfigLoader& config_loader, std::string const& name)
{
    std::string upper_name = name;
    std::transform(upper_name.begin(), upper_name.end(), upper_name.begin(), [](unsigned char const c) { return std::toupper(c); });

    auto const& config_file = config_loader.open_config_cached(CONFIG_FILE);
    auto        sequence    = read_to<TextureSequenceInfo>::from_ini(config_file, upper_name);
    if (sequence.texture_file.empty()) { sequence.texture_file = std::format("{}.tga", name); }

    return sequence;
}

}  // namespace storm::texture_sequence
