#include "ini_file.h"

#include <cassert>
#include <string_view>

#include <libs/core/core.h>
#include <libs/filesystem/v_file_service.h>

using namespace storm;

namespace
{

constexpr char WHITESPACE_CHARACTERS[] = "\t ";
constexpr char NEWLINE_CHARACTERS[]    = "\r\n";

constexpr char INI_SECTION_START   = '[';
constexpr char INI_SECTION_END     = ']';
constexpr char INI_COMMENT_START[] = ";#";

constexpr char KEY_VALUE_SEP   = '=';
constexpr char KEY_VALUE_END[] = ";#";

std::string get_section_name(std::string_view const& str, std::string const& ini_file_name, size_t const ini_file_line)
{
    // Read section name inside brackets: [<section_name>]
    assert(str.at(0) == INI_SECTION_START);
    auto const section_end = str.find_first_of(INI_SECTION_END);
    if (section_end == std::string_view::npos) {
        core->Trace(
            "%s:%d: syntax error: no closing bracket (']') - section name ends unexpectedly\n\t%s",
            ini_file_name.c_str(),
            ini_file_line,
            str.data());
        return {};
    }

    return std::string(str.substr(1, section_end - 1));  // Skip '[' and ']' characters
}

std::string trim(std::string_view const& str)
{
    auto const start = str.find_first_not_of(WHITESPACE_CHARACTERS);
    auto const end   = str.find_last_not_of(WHITESPACE_CHARACTERS);
    if (start == std::string_view::npos || end == std::string_view::npos) { return {}; }  // If there is only spaces return empty string

    return std::string(str.substr(start, end - start + 1));  // Add 1 to preserve last character
}

std::pair<std::string, std::string>
get_key_value_pair(std::string_view const& str, std::string const& ini_file_name, size_t const ini_file_line)
{
    auto const key_end = str.find_first_of(KEY_VALUE_SEP);  // Find '=' sign
    if (key_end == std::string_view::npos) {
        core->Trace("%s:%d: syntax error: no '=' after key name\n\t%s", ini_file_name.c_str(), ini_file_line, str.data());
        return std::make_pair(std::string {}, std::string {});
    }

    auto const value_start = key_end + 1;                       // Skip '='
    auto const value_end   = str.find_first_of(KEY_VALUE_END);  // Find comment start (or npos)

    auto const key = str.substr(0, key_end);  // Read the key from the start until '='
    auto const value = str.substr(  // Read the value after '=' until the end of the line or start of the comment
        value_start,
        value_end != std::string_view::npos
            ? value_end - value_start
            : std::string_view::npos);

    return std::make_pair(trim(key), trim(value));
}

std::pair<std::string, IniFile::Section>
read_section(std::string_view const& str, size_t& offset, std::string const& file_name, size_t& file_line)
{
    std::string      section_name = {};  // Empty section means global keys at the start of the file
    IniFile::Section section      = {};

    bool is_section_started = false;
    bool is_section_ended   = false;
    while (offset < str.size() && !is_section_ended) {
        // Skip spaces and tabs at the line start
        auto const line_start = str.find_first_not_of(WHITESPACE_CHARACTERS, offset);
        if (line_start == std::string_view::npos) { break; }

        auto const line_end = str.find_first_of(NEWLINE_CHARACTERS, line_start);
        auto const line     = str.substr(line_start, line_end != std::string_view::npos ? line_end - line_start : std::string_view::npos);

        if (!line.empty()) {
            switch (line.at(0)) {
            case INI_COMMENT_START[0]: [[fallthrough]];
            case INI_COMMENT_START[1]: break;  // Skip comment line

            case INI_SECTION_START:
                if (!is_section_started) {
                    section_name       = get_section_name(line, file_name, file_line);
                    is_section_started = true;  // Section name is parsed, section is started
                } else {
                    is_section_ended = true;  // If there is another section start, then we're done with current section
                }
                break;

            default: {
                auto&& [key, value] = get_key_value_pair(line, file_name, file_line);
                section.emplace(std::move(key), std::move(value));
                is_section_started = true;  // If there is no section name, any read key means global section
            } break;
            }
        }

        // Keep offset if section is ended as we need to read another section from that place
        if (!is_section_ended) {
            // Move to next line. This statement allows us to skip both CRLF and LF
            offset = str.find_first_of('\n', line_end) + 1;
            ++file_line;
        }
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
    size_t     line    = 0;

    while (offset < content.size()) {
        auto const [section_name, section_table] = read_section(content, offset, file_path.string(), line);
        m_table.emplace(section_name, section_table);
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
