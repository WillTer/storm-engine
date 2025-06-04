#pragma once

#include <string>

#include "../../i_common/mem_file.h"

class DataFloat
{
    std::string Name;

    float Value;

public:
    // constructor / destructor
    DataFloat();
    virtual ~DataFloat();

    // Get value
    float GetValue() const;

    // Set value
    void SetValue(float val);

    void Load(MemFile* File);
    void Write(MemFile* File) const;

    void        SetName(char const* szName);
    char const* GetName() const;
};
