#pragma once

#include <string>

#include <libs/core/attributes.h>

namespace storm
{

void removeCarriageReturn(std::string& str);

void parseOptions(std::string_view const& str, ATTRIBUTES& attribute);

}  // namespace storm
