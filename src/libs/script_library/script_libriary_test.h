#pragma once
#include <libs/core/script_libriary.h>

class ScriptLibraryTest: public SCRIPT_LIBRIARY
{
public:
    ScriptLibraryTest() {};

    ~ScriptLibraryTest() override {};
    bool Init() override;
};
