#include "options_parser.hpp"

#include <algorithm>

namespace storm
{

namespace
{

constexpr char const VALUE_SEPARATOR  = u8'=';
constexpr char const OPTION_SEPARATOR = u8'\n';

}  // namespace

void removeCarriageReturn(std::string& str)
{
    auto newLength = std::remove(str.begin(), str.end(), u8'\r');
    if (newLength != str.end()) { str.erase(newLength, str.end()); }
}

void parseOptions(std::string_view const& str, ATTRIBUTES& attribute)
{
    size_t option_offset = 0;
    for (;;) {
        option_offset = str.find_first_not_of(" \t\n", option_offset);
        if (option_offset == std::string_view::npos) { break; }

        size_t const name_offset = option_offset;
        size_t const name_end    = str.find_first_of(VALUE_SEPARATOR, option_offset);
        if (name_end == std::string_view::npos) { break; }
        size_t const name_length = name_end - name_offset;

        size_t const value_offset = name_offset + name_length + 1;
        size_t       value_end    = str.find_first_of(OPTION_SEPARATOR, value_offset);
        if (value_end == std::string_view::npos) { value_end = str.length(); }
        size_t const value_length = value_end - value_offset;

        // TODO: Update ATTRIBUTES to support string_view so we don't have to allocate memory
        auto const name  = std::string(str.substr(name_offset, name_length));
        auto const value = std::string(str.substr(value_offset, value_length));

        ATTRIBUTES* pA = attribute.CreateSubAClass(&attribute, name.c_str());
        pA->SetValue(value);

        option_offset = value_offset + value_length;
    }
}

}  // namespace storm
