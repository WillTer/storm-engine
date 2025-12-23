#include "sound_alias.h"

#include <ranges>

#include <libs/core/core.h>

#include "i_config_loader.h"
#include "ini_file.h"
#include "ini_helpers.h"

using namespace storm;

namespace
{

constexpr float DEFAULT_PROBABILITY = 1.0F;

}  // namespace

template <>
struct storm::read_to<SoundAlias> {
    static SoundAlias from_ini(IniFile const& ini, std::string const& section)
    {
        ProbabilityTable<std::string> files = {};

        // TODO: syntax check
        auto [name_begin, name_end] = ini.get_section(section).equal_range("name");
        std::for_each(name_begin, name_end, [&](std::pair<std::string, std::string> const& pair) {
            auto const& [_, name] = pair;
            // Check if it's just name or name with probability separated by comma
            auto const comma = name.find_first_of(',');

            try {
                float const       probability = comma == std::string::npos ? DEFAULT_PROBABILITY : std::stof(name.substr(comma + 1));
                std::string const file_name   = comma == std::string::npos ? name : name.substr(0, comma);

                files.emplace(probability, file_name);
            } catch (std::invalid_argument const& e) {
                core->Trace("IniFile::read_to<SoundAlias>() can't parse section \"%s\", name value \"%s\"", section.c_str(), name.c_str());
            }
        });

        return {
            .min_distance = ini.find_or(section, "minDistance", -1.0F),
            .max_distance = ini.find_or(section, "maxDistance", -1.0F),
            .volume       = ini.find_or(section, "volume", -1.0F),
            .files        = std::move(files),
        };
    }
};

std::unordered_map<std::string, SoundAlias> sound_alias::aliases(std::filesystem::path const& file)
{
    auto const& config_file = config_loader->open_config_cached(file);

    std::unordered_map<std::string, SoundAlias> aliases = {};
    for (auto const& name: config_file.get_sections() | std::views::keys) {
        aliases.emplace(name, read_to<SoundAlias>::from_ini(config_file, name));
    }

    return aliases;
}
