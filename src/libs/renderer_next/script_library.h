#pragma once

#include <libs/core/core.h>
#include <libs/core/s_import_func.h>
#include <libs/core/script_libriary.h>

uint32_t DX9SetTexturePath(VS_STACK* pS);
uint32_t RPrint(VS_STACK* pS);
uint32_t SetGlowParams(VS_STACK* pS);
uint32_t slGetTexture(VS_STACK* pS);
uint32_t slReleaseTexture(VS_STACK* pS);

class DX9RenderScriptLibrary: public SCRIPT_LIBRIARY
{
public:
    DX9RenderScriptLibrary() {}

    ~DX9RenderScriptLibrary() override {}

    bool Init() override
    {
        IFUNCINFO sIFuncInfo;

        sIFuncInfo.nArguments       = 2;
        sIFuncInfo.pFuncName        = "SetTexturePath";
        sIFuncInfo.pReturnValueName = "int";
        sIFuncInfo.pFuncAddress     = DX9SetTexturePath;
        core->SetScriptFunction(&sIFuncInfo);

        sIFuncInfo.nArguments       = 3;
        sIFuncInfo.pFuncName        = "RPrint";
        sIFuncInfo.pReturnValueName = "int";
        sIFuncInfo.pFuncAddress     = RPrint;
        core->SetScriptFunction(&sIFuncInfo);

        sIFuncInfo.nArguments       = 3;
        sIFuncInfo.pFuncName        = "SetGlowParams";
        sIFuncInfo.pReturnValueName = "int";
        sIFuncInfo.pFuncAddress     = SetGlowParams;
        core->SetScriptFunction(&sIFuncInfo);

        sIFuncInfo.nArguments       = 1;
        sIFuncInfo.pFuncName        = "GetTexture";
        sIFuncInfo.pReturnValueName = "int";
        sIFuncInfo.pFuncAddress     = slGetTexture;
        core->SetScriptFunction(&sIFuncInfo);

        sIFuncInfo.nArguments       = 1;
        sIFuncInfo.pFuncName        = "ReleaseTexture";
        sIFuncInfo.pReturnValueName = "void";
        sIFuncInfo.pFuncAddress     = slReleaseTexture;
        core->SetScriptFunction(&sIFuncInfo);

        return true;
    }
};
