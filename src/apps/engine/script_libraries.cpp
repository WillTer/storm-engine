#include <libs/core/vma.hpp>
#include <libs/location/location_script_lib.h>
#include <libs/renderer/s_device.h>
#include <libs/rigging/script_func.h>
#include <libs/script_library/script_libriary_test.h>
#include <libs/steam_api/steam_api_script_lib.hpp>
#include <libs/xinterface/string_service/str_service.h>

CREATE_SCRIPT_LIBRARY(DX9RenderScriptLibrary)

CREATE_SCRIPT_LIBRARY(ScriptLocationLibrary)
CREATE_SCRIPT_LIBRARY(ScriptInterfaceFunctions)
CREATE_SCRIPT_LIBRARY(ScriptLibraryTest)
CREATE_SCRIPT_LIBRARY(ScriptRiggingFiles)

namespace steamapi
{
CREATE_SCRIPT_LIBRARY(SteamApiScriptLib)
}
