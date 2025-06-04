#pragma once

#include <vector>

#include "core_impl.h"

struct BREAKPOINT_DESC {
    uint32_t nLineNumber;
    char*    pFileName;
};

class BREAKPOINTS_TABLE
{
    uint32_t                     nPoints;
    std::vector<BREAKPOINT_DESC> pTable;
    char                         ProjectName[MAX_PATH];
    bool                         bReleased;

public:
    BREAKPOINTS_TABLE();
    ~BREAKPOINTS_TABLE();
    bool ReadProject(char const* filename);
    void AddBreakPoint(char const* filename, uint32_t line);
    void DelBreakPoint(char const* filename, uint32_t line);
    bool Find(char const* filename, uint32_t line);
    void FlipBreakPoint(char const* filename, uint32_t line);
    void Release();

    bool CanBreak()
    {
        if (nPoints > 0) return true;
        return false;
    };
    void UpdateProjectFile();
};
