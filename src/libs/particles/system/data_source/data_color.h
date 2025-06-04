#pragma once

#include <string>
#include <vector>

#include "../../i_common/color_vertex.h"
#include "../../i_common/mem_file.h"

class DataColor
{
    std::string Name;

    Color const ZeroColor;

    std::vector<ColorVertex> ColorGraph;

public:
    // constructor / destructor
    DataColor();
    virtual ~DataColor();

    // Get the value (Current time, Total lifetime, Random factor [0..1])
    Color GetValue(float Time, float LifeTime, float K_rand);

    // Sets the "default"
    // two indices, Min = Max = Value
    void SetDefaultValue(Color const& Value);

    // Set values
    void SetValues(ColorVertex const* Values, uint32_t Count);

    // Get count of values
    uint32_t GetValuesCount() const;

    // Get min. value (by index)
    Color const& GetMinValue(uint32_t Index);

    // Get max. value (by index)
    Color const& GetMaxValue(uint32_t Index);

    void Load(MemFile* File);
    void Write(MemFile* File);

    void        SetName(char const* szName);
    char const* GetName() const;

    ColorVertex const& GetByIndex(uint32_t Index);
};
