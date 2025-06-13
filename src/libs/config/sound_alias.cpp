#include "sound_alias.h"

#include <libs/core/core.h>
#include <toml.hpp>

using namespace storm;

namespace
{

constexpr float DEFAULT_PROBABILITY = 1.0F;

}  // namespace

namespace toml
{

template <>
struct from<storm::SoundAlias> {
    static storm::SoundAlias from_toml(toml::value const& v)
    {
        // No sound_files - no alias
        if (!v.is_table() || !v.contains("sound_files") || !v.at("sound_files").is_array()) { return {}; }

        storm::ProbabilityTable<std::string> files = {};
        for (auto const& file: v.at("sound_files").as_array()) {
            if (!file.is_table() || !file.contains("name")) { return {}; }

            auto const probability = toml::find_or(file, "probability", DEFAULT_PROBABILITY);
            auto const name        = toml::find_or<std::string>(file, "name", "");

            if (name.empty()) { continue; }

            files.emplace(probability, name);
        }

        return {
            .min_distance = toml::find_or(v, "min_distance", -1.0F),
            .max_distance = toml::find_or(v, "max_distance", -1.0F),
            .volume       = toml::find_or(v, "volume", -1.0F),
            .files        = std::move(files),
        };
    }
};

}  // namespace toml

std::unordered_map<std::string, SoundAlias> sound_alias::aliases(IConfigLoader& config_loader, std::filesystem::path const& file)
{
    auto const config_file = config_loader.open_config_cached(file);
    if (!config_file.is_table()) {
        core.Trace("There are no aliases in file \"%s\"", file.string().c_str());
        return {};
    }

    std::unordered_map<std::string, SoundAlias> aliases = {};
    for (auto const& [name, value]: config_file.as_table()) {
        aliases.emplace(name, toml::get<SoundAlias>(value));
    }

    return aliases;
}
