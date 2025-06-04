#pragma once

#include <string>

#include "../../i_common/mem_file.h"

class DataString
{
    std::string Name;

    std::string Value;

public:
    // constructor / destructor
    DataString();
    virtual ~DataString();

    // Get value
    char const* GetValue() const;

    // Set value
    void SetValue(char const* val);

    void Load(MemFile* File);
    void Write(MemFile* File) const;

    void        SetName(char const* szName);
    char const* GetName() const;
};
