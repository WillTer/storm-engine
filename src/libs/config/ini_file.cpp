#include "ini_file.h"

#include <string_view>

#include <libs/core/core.h>
#include <libs/filesystem/v_file_service.h>

using namespace storm;

namespace
{

constexpr std::string_view DEFAULT_SECTION_NAME = "";

constexpr char WHITESPACE_CHARACTERS[] = "\r\n\t ";
constexpr char NEWLINE_CHARACTERS[]    = "\r\n";

constexpr char INI_SECTION_START   = '[';
constexpr char INI_SECTION_END     = ']';
constexpr char INI_COMMENT_START[] = ";#";
constexpr char INI_VALUE_START     = '=';
constexpr char INI_VALUE_END[]     = "\r\n;#";

std::pair<std::string_view, IniFile::Section> read_section(std::string_view const& str, size_t& offset)
{
    auto const skip_to_next_line = [&str](size_t current_offset) {
        current_offset = str.find_first_of(NEWLINE_CHARACTERS, current_offset);
        if (current_offset != std::string_view::npos) { current_offset = str.find_first_not_of(WHITESPACE_CHARACTERS, current_offset); }
        return current_offset;
    };

    auto const trim = [](std::string_view const& s) -> std::string {
        auto const start = s.find_first_not_of(WHITESPACE_CHARACTERS);
        auto const end   = s.find_last_not_of(WHITESPACE_CHARACTERS);
        if (start == std::string_view::npos || end == std::string_view::npos) { return {}; }  // If there is only spaces return empty string

        return std::string(s.substr(start, std::min(end - start + 1, s.size() - start)));  // Add 1 to preserve last character
    };

    std::string_view section_name = DEFAULT_SECTION_NAME;
    if (str.at(offset) == INI_SECTION_START) {  // Read section name if we at the section start
        auto const section_end = str.find_first_of(INI_SECTION_END, offset);
        section_name           = str.substr(offset + 1, section_end - (offset + 1));

        // Skip until next line start after we read section name
        offset = skip_to_next_line(section_end);
    }

    IniFile::Section section = {};
    while (offset < str.size() && str.at(offset) != INI_SECTION_START) {
        if (str.at(offset) == INI_COMMENT_START[0] || str.at(offset) == INI_COMMENT_START[1]) {
            offset = skip_to_next_line(offset);
            continue;
        }

        auto const value_start = str.find_first_of(INI_VALUE_START, offset);  // Find '=' sign
        if (value_start == std::string_view::npos) {
            core->Trace("Ini file syntax error: no '=' after key name \"%s\"", str.substr(offset).data());
            offset = skip_to_next_line(offset);
            continue;
        }

        auto const value_end = str.find_first_of(INI_VALUE_END, value_start);  // Find newline or comment start after equals

        auto const key = str.substr(offset, value_start - offset);  // Read the key from start until '='
        auto const value = str.substr(  // Read the value from '=' (skip equals sign itself) until line (or file) end
            value_start + 1,
            value_end != std::string_view::npos
                ? value_end - (value_start + 1)
                : std::string_view::npos);

        section.emplace(trim(key), trim(value));

        offset = skip_to_next_line(offset);  // Skip to next line start
    }

    return std::make_pair(section_name, section);
}

}  // namespace

IniFile::IniFile(std::filesystem::path const& file_path)
{
    if (!fio->exists(file_path)) {
        core->Trace("IniFile::IniFile(): file not found (\"%s\")", fio->transform_path(file_path).string().c_str());
        return;
    }

    std::vector<char> file_data = {};
    if (!fio->read_file_to_mem(file_path, file_data)) {
        core->Trace("IniFile::IniFile(): file read error (\"%s\")", fio->transform_path(file_path).string().c_str());
        return;
    }

    auto const content = std::string_view(file_data.begin(), file_data.end());
    size_t     offset  = 0;

    while (offset < content.size()) {
        auto const [section_name, section_table] = read_section(content, offset);
        m_table.emplace(std::string(section_name), section_table);
    }
}

std::optional<std::string> IniFile::try_get_string(std::string const& section, std::string const& key) const
{
    if (!m_table.contains(section) || !m_table.at(section).contains(key)) { return std::nullopt; }

    auto const& s           = m_table.at(section);
    auto const [begin, end] = s.equal_range(key);

    return begin->second;
}

std::string IniFile::get_string(std::string const& section, std::string const& key) const
{
    if (auto const value = try_get_string(section, key); value.has_value()) { return value.value(); }

    throw std::runtime_error(std::format("IniFile: key (\"{}\") or section (\"{}\") not found ", key, section));
}

std::vector<std::string> IniFile::get_vector(std::string const& section, std::string const& key) const
{
    if (!m_table.contains(section) || !m_table.at(section).contains(key)) { return {}; }

    auto const& s           = m_table.at(section);
    auto const [begin, end] = s.equal_range(key);

    auto vec = std::vector<std::string> {};
    for (auto it = begin; it != end; ++it) {
        vec.emplace_back(it->second);
    }

    return vec;
}

bool IniFile::is_empty() const
{
    return m_table.empty();
}

IniFile::Section const& IniFile::get_section(std::string const& section) const
{
    return m_table.at(section);
}

std::unordered_map<std::string, IniFile::Section> const& IniFile::get_sections() const
{
    return m_table;
}
