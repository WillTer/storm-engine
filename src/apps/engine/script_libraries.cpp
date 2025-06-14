

#include <libs/core/vma.hpp>
#include <libs/location/location_script_lib.h>
#include <libs/renderer/s_device.h>
#include <libs/rigging/script_func.h>
#include <libs/script_library/script_libriary_test.h>
#include <libs/steam_api/steam_api_script_lib.hpp>
#include <libs/xinterface/string_service/str_service.h>

CREATE_SCRIPTLIBRIARY(DX9RENDER_SCRIPT_LIBRIARY)

CREATE_SCRIPTLIBRIARY(ScriptLocationLibrary)
CREATE_SCRIPTLIBRIARY(SCRIPT_INTERFACE_FUNCTIONS)
CREATE_SCRIPTLIBRIARY(SCRIPT_LIBRIARY_TEST)
CREATE_SCRIPTLIBRIARY(SCRIPT_RIGGING_FILES)

namespace steamapi
{
CREATE_SCRIPTLIBRIARY(SteamApiScriptLib)
}
