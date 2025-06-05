#include <libs/math/math_inlines.h>
#include <libs/util/debug-trap.h>

#include "island.h"

namespace
{

uint32_t Number2Shift(uint32_t dwNumber)
{
    for (uint32_t i = 0; i < 31; i++)
        if (static_cast<uint32_t>(1 << i) == dwNumber) return i;
    return 0;
}

}  // namespace

MapZipper::MapZipper()
{
    pWordTable = nullptr;
    pRealData  = nullptr;
}

MapZipper::~MapZipper()
{
    UnInit();
}

void MapZipper::UnInit()
{
    STORM_DELETE(pWordTable);
    free(pRealData);
}

void MapZipper::DoZip(uint8_t* pSrc, uint32_t _dwSizeX)
{
    uint32_t i, j, k, x, y, xx, yy;

    UnInit();

    uint32_t dwRealIndex = 0;

    dwSizeX = _dwSizeX;

    dwBlockSize  = 8;
    dwBlockShift = Number2Shift(dwBlockSize);

    dwDX = dwSizeX >> dwBlockShift;

    dwShiftNumBlocksX = Number2Shift(dwDX);

    pWordTable = new uint16_t[dwDX * dwDX];
    pRealData  = static_cast<uint8_t*>(malloc(dwSizeX * dwSizeX));
    for (i = 0; i < dwDX * dwDX; i++) {
        y                  = i / dwDX;
        x                  = i - y * dwDX;
        auto const dwStart = (y << dwBlockShift) * dwSizeX + (x << dwBlockShift);

        auto    bTest = true;
        uint8_t byTest;
        for (j = 0; j < dwBlockSize * dwBlockSize; j++) {
            yy               = j >> dwBlockShift;
            xx               = j - (yy << dwBlockShift);
            auto const byRes = pSrc[dwStart + yy * dwSizeX + xx];
            if (j == 0) byTest = byRes;
            if (byTest != byRes) {
                bTest         = false;
                pWordTable[i] = static_cast<uint16_t>(dwRealIndex);
                for (k = 0; k < dwBlockSize * dwBlockSize; k++) {
                    yy                                                     = k >> dwBlockShift;
                    xx                                                     = k - (yy << dwBlockShift);
                    pRealData[dwRealIndex * dwBlockSize * dwBlockSize + k] = pSrc[dwStart + yy * dwSizeX + xx];
                }
                dwRealIndex++;
                break;
            }
        }
        if (bTest) pWordTable[i] = static_cast<uint16_t>(0x8000) | static_cast<uint16_t>(byTest);
    }
    dwNumRealBlocks = dwRealIndex;
    pRealData       = static_cast<uint8_t*>(realloc(pRealData, dwRealIndex * dwBlockSize * dwBlockSize));

    for (y = 0; y < _dwSizeX; y++)
        for (x = 0; x < _dwSizeX; x++) {
            if (Get(x, y) != pSrc[x + y * _dwSizeX]) psnip_trap();
        }
}

uint8_t MapZipper::Get(uint32_t dwX, uint32_t dwY)
{
    if (!pWordTable) return 255;
    auto const wRes = pWordTable[((dwY >> dwBlockShift) << dwShiftNumBlocksX) + (dwX >> dwBlockShift)];
    if (wRes & 0x8000) return static_cast<uint8_t>(wRes & 0xFF);
    auto const x = dwX - ((dwX >> dwBlockShift) << dwBlockShift);
    auto const y = dwY - ((dwY >> dwBlockShift) << dwBlockShift);

    auto const byRes = pRealData[((static_cast<uint32_t>(wRes) << dwBlockShift) << dwBlockShift) + (y << dwBlockShift) + x];

    return byRes;
}

bool MapZipper::Load(std::string sFileName)
{
    UnInit();

    auto stream = std::ifstream(sFileName.c_str(), std::ios::binary);
    if (!stream.is_open()) { return false; }
    stream.read(reinterpret_cast<char*>(&dwSizeX), sizeof(dwSizeX));
    stream.read(reinterpret_cast<char*>(&dwDX), sizeof(dwDX));
    stream.read(reinterpret_cast<char*>(&dwBlockSize), sizeof(dwBlockSize));
    stream.read(reinterpret_cast<char*>(&dwBlockShift), sizeof(dwBlockShift));
    stream.read(reinterpret_cast<char*>(&dwShiftNumBlocksX), sizeof(dwShiftNumBlocksX));
    stream.read(reinterpret_cast<char*>(&dwNumRealBlocks), sizeof(dwNumRealBlocks));
    pWordTable = new uint16_t[dwDX * dwDX];
    stream.read(reinterpret_cast<char*>(pWordTable), sizeof(uint16_t) * dwDX * dwDX);
    pRealData = static_cast<uint8_t*>(malloc(dwNumRealBlocks * dwBlockSize * dwBlockSize));
    stream.read(reinterpret_cast<char*>(pRealData), sizeof(uint8_t) * dwNumRealBlocks * dwBlockSize * dwBlockSize);
    return true;
}

bool MapZipper::Save(std::string sFileName)
{
    auto stream = std::ofstream(sFileName.c_str(), std::ios::binary);
    if (!stream.is_open()) { return false; }
    stream.write(reinterpret_cast<char*>(&dwSizeX), sizeof(dwSizeX));
    stream.write(reinterpret_cast<char*>(&dwDX), sizeof(dwDX));
    stream.write(reinterpret_cast<char*>(&dwBlockSize), sizeof(dwBlockSize));
    stream.write(reinterpret_cast<char*>(&dwBlockShift), sizeof(dwBlockShift));
    stream.write(reinterpret_cast<char*>(&dwShiftNumBlocksX), sizeof(dwShiftNumBlocksX));
    stream.write(reinterpret_cast<char*>(&dwNumRealBlocks), sizeof(dwNumRealBlocks));
    stream.write(reinterpret_cast<char*>(pWordTable), sizeof(uint16_t) * dwDX * dwDX);
    stream.write(reinterpret_cast<char*>(pRealData), sizeof(uint8_t) * dwNumRealBlocks * dwBlockSize * dwBlockSize);
    return true;
}
