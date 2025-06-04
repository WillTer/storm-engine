#pragma once

#include <libs/core/script_libriary.h>

namespace steamapi
{
class SteamApiScriptLib: public SCRIPT_LIBRIARY
{
public:
    bool Init() override;
};
}  // namespace steamapi
