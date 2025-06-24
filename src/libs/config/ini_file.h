#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ini_helpers.h"

namespace storm
{

struct TextFileAsset;
class IniFile
{
public:
    using Section = std::unordered_multimap<std::string, std::string>;

    explicit IniFile(TextFileAsset const& asset);

    bool contains(std::string const& section, std::string const& key) const;

    std::optional<std::string> try_get_string(std::string const& section, std::string const& key) const;
    std::string                get_string(std::string const& section, std::string const& key) const;

    std::vector<std::string> get_vector(std::string const& section, std::string const& key) const;

    template <typename T>
    T find(std::string const& section, std::string const& key) const
    {
        return convert_to<T>::from_string(get_string(section, key));
    }

    template <typename T>
    T find_or(std::string const& section, std::string const& key, T const& def_value) const
    {
        auto const val = try_get_string(section, key);
        return val.has_value() ? convert_to<T>::from_string(val.value()) : def_value;
    }

    bool is_empty() const;

    Section const& get_section(std::string const& section) const;

    std::unordered_map<std::string, Section> const& get_sections() const;

private:
    std::unordered_map<std::string, Section> m_table;
};

}  // namespace storm
