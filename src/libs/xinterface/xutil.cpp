#include <cstring>

#include <libs/core/entity.h>
#include <libs/util/storm_assert.h>
#include <libs/util/string_compare.hpp>

#include "libs/renderer_next/types.h"

#include "xdefines.h"

//////////////////////////////////////////////
///    DEFINE GLOBAL DATA                 ////
////////////////////////////////////////////////////////////////////////////////////////////////////
entid_t g_idInterface;

COMMANDDESCR pCommandsList[COMMAND_QUANTITY] = {
    {"activate", ACTION_ACTIVATE},
    {"deactivate", ACTION_DEACTIVATE},
    {"select", ACTION_SELECT},
    {"rightstep", ACTION_RIGHTSTEP},
    {"leftstep", ACTION_LEFTSTEP},
    {"upstep", ACTION_UPSTEP},
    {"downstep", ACTION_DOWNSTEP},
    {"speedleft", ACTION_SPEEDLEFT},
    {"speedright", ACTION_SPEEDRIGHT},
    {"speedup", ACTION_SPEEDUP},
    {"speeddown", ACTION_SPEEDDOWN},
    {"click", ACTION_MOUSECLICK},
    {"dblclick", ACTION_MOUSEDBLCLICK},
    {"rclick", ACTION_MOUSERCLICK},
    {"dblrclick", ACTION_MOUSERDBLCLICK}};

////////////////////////////////////////////////////////////////////////////////////////////////////
//////  END GLOBAL DATA                   ////
//////////////////////////////////////////////

int FindCommand(char const* comName)
{
    int i;
    for (i = 0; i < COMMAND_QUANTITY; i++)
        if (storm::iEquals(comName, pCommandsList[i].sName)) break;
    if (i == COMMAND_QUANTITY) return -1;
    return i;
}

int FindCommand(int comID)
{
    int i;
    for (i = 0; i < COMMAND_QUANTITY; i++)
        if (pCommandsList[i].code == comID) break;
    if (i == COMMAND_QUANTITY) return -1;
    return i;
}

uint32_t ColorInterpolate(uint32_t sCol, uint32_t dCol, float m)
{
    int a, r, g, b;
    a = ALPHA(sCol);
    r = RED(sCol);
    g = GREEN(sCol);
    b = GREEN(sCol);
    int ad, rd, gd, bd;
    ad = ALPHA(dCol);
    rd = RED(dCol);
    gd = GREEN(dCol);
    bd = BLUE(dCol);

    a += static_cast<int>((ad - a) * m);
    r += static_cast<int>((rd - r) * m);
    g += static_cast<int>((gd - g) * m);
    b += static_cast<int>((bd - b) * m);
    return storm::Color {static_cast<uint8_t>(a), static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b)}.to_hex();
}

void DublicateString(char*& pDstStr, char const* pSrcStr)
{
    if (!pSrcStr || pSrcStr[0] == 0)  // boal fix
    {
        pDstStr = nullptr;
    } else {
        delete[] pDstStr;
        auto const len = strlen(pSrcStr) + 1;
        pDstStr        = new char[len];
        Assert(pDstStr);
        memcpy(pDstStr, pSrcStr, len);
    }
}
