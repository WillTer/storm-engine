#include "dialog_utils.hpp"

namespace storm::dialog
{

std::vector<int32_t> SplitIntoPages(size_t const line_count, int32_t const page_height, std::vector<int32_t> const& forced_breaks)
{
    std::vector<int32_t> page_ends;
    int32_t              current_page_break = 0;
    for (int32_t forced_break: forced_breaks) {
        if (forced_break > line_count) { break; }

        while (forced_break > current_page_break + page_height) {
            current_page_break += page_height;
            page_ends.push_back(current_page_break);
        }

        current_page_break = forced_break;
        page_ends.push_back(current_page_break);
    }
    while (current_page_break + page_height < line_count) {
        current_page_break += page_height;
        page_ends.push_back(current_page_break);
    }
    return page_ends;
}

void AddToStringArrayLimitedByWidth(
    std::string_view const&       text,
    int32_t                       nLimitWidth,
    std::vector<std::string>&     asOutTextList,
    GetStringWidthFunction const& get_string_width)
{
    if (nLimitWidth < 20) nLimitWidth = 20;

    size_t           current_offset = 0;
    std::string_view current_span   = text;
    for (;;) {
        size_t const next_space = current_span.find_first_of(" \\", current_offset);

        if (next_space != std::string_view::npos) {
            std::string_view const text_section = current_span.substr(0, next_space);
            int32_t const          nW           = get_string_width(text_section);
            if (nW > nLimitWidth) {
                size_t const last_space = text_section.find_last_of(" \\");
                asOutTextList.emplace_back(text_section.substr(0, last_space));

                if (current_span[last_space] == '\\' && current_span.size() > last_space + 1 && current_span[last_space + 1] == 'n') {
                    current_span   = current_span.substr(last_space);
                    current_offset = 0;
                } else {
                    current_span   = current_span.substr(last_space + 1u);
                    current_offset = 0;
                }
            } else if (current_span[next_space] == '\\' && current_span.size() > next_space + 1 && current_span[next_space + 1] == 'n') {
                asOutTextList.emplace_back(text_section.substr(0, next_space));
                current_span   = current_span.substr(next_space + 2u);
                current_offset = 0;
            } else {
                current_offset = next_space + 1;
            }
        } else {
            asOutTextList.emplace_back(current_span);
        }

        if (next_space == std::string_view::npos) { break; }
    }
}

}  // namespace storm::dialog
