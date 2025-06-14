#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include <libs/util/probability_table.hpp>

namespace storm
{

struct SoundAlias {
    float min_distance;
    float max_distance;
    float volume;

    storm::ProbabilityTable<std::string> files;
};

namespace sound_alias
{

std::unordered_map<std::string, SoundAlias> aliases(std::filesystem::path const& file);

}

}  // namespace storm
