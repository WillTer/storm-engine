#pragma once

#include <string>
#include <vector>

#include <libs/math/math3d.h>

#include "../../i_common/mem_file.h"

class DataUV
{
    std::string Name;

    std::vector<Vector4> Frames;

public:
    // constructor / destructor
    DataUV();
    virtual ~DataUV();

    // Get the value [x, y = UV1; z, w = UV2]
    Vector4 const& GetValue(uint32_t FrameNum);

    // Set values
    void SetValues(Vector4 const* _Frames, uint32_t FramesCount);

    // Get the number of frames
    uint32_t GetFrameCount() const;

    void Load(MemFile* File);
    void Write(MemFile* File);

    void        SetName(char const* szName);
    char const* GetName() const;
};
