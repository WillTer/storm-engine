#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace storm::dialog
{

using GetStringWidthFunction = std::function<int32_t(std::string_view const& text)>;

std::vector<int32_t> SplitIntoPages(size_t const line_count, int32_t const page_height, std::vector<int32_t> const& forced_breaks = {});

void AddToStringArrayLimitedByWidth(
    std::string_view const&       text,
    int32_t                       nLimitWidth,
    std::vector<std::string>&     asOutTextList,
    GetStringWidthFunction const& get_string_width);

}  // namespace storm::dialog
