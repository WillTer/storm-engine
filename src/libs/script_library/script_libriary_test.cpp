#include "script_libriary_test.h"

#include <libs/core/vma.hpp>

#include "vano_files.h"

CREATE_SCRIPTLIBRIARY(SCRIPT_LIBRIARY_TEST)

bool SCRIPT_LIBRIARY_TEST::Init()
{
    Vano_Init();

    return true;
};
