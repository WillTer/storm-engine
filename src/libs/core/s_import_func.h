#pragma once

#include <cstdint>

enum FuncResult : uint32_t { IFUNCRESULT_OK = 0U, IFUNCRESULT_FAILED = 0xffffffffU };

#define INVALID_FUNCHANDLE 0xffffffffU

#include <cstdint>

class VS_STACK;
using SIMPORTFUNC = uint32_t (*)(VS_STACK*);

struct IFUNCINFO {
    IFUNCINFO() : nArguments(0)
    {
        pFuncName        = nullptr;
        pReturnValueName = nullptr;
        pFuncAddress     = nullptr;
        pDeclFileName    = nullptr;
        nDeclLine        = 0;
    };
    char const* pFuncName;
    char const* pReturnValueName;
    uint32_t    nArguments;
    SIMPORTFUNC pFuncAddress;
    char const* pDeclFileName;
    uint32_t    nDeclLine;
};
