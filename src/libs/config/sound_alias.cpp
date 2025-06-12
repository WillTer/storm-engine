#include "sound_alias.h"

using namespace storm;

namespace
{

constexpr float DEFAULT_PROBABILITY = 1.0F;

}

std::unordered_map<std::string, SoundAlias> sound_alias::aliases(IConfigLoader& config_loader, std::filesystem::path const& file)
{
    auto const config_file = config_loader.open_config_cached(file);

    std::unordered_map<std::string, SoundAlias> aliases = {};

    for (auto const& [section_name, section]: config_file->as_table()) {
        if (!section.is_table()) { continue; }
        auto const table = section.as_table();

        aliases.emplace(
            section_name,
            SoundAlias {
                .min_distance = storm::config::find(table, "min_distance", -1.0F),
                .max_distance = storm::config::find(table, "max_distance", -1.0F),
                .volume       = storm::config::find(table, "volume", -1.0F),
            });

        SoundAlias& alias       = aliases[std::string(section_name)];
        auto const  sound_files = storm::config::find<std::vector<storm::ConfigValue>>(table, "sound_files");
        for (auto const& sound_file: sound_files) {
            if (!sound_file.is_table()) { break; }

            auto const table       = sound_file.as_table();
            auto const name        = storm::config::find<std::string>(table, "name");
            auto const probability = storm::config::find(table, "probability", DEFAULT_PROBABILITY);

            alias.files.emplace(probability, name);
        }
    }

    return aliases;
}
