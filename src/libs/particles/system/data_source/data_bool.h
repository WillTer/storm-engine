#pragma once

#include <string>

#include "../../i_common/mem_file.h"

class DataBool
{
    std::string Name;

    bool Value;

public:
    // constructor / destructor
    DataBool();
    virtual ~DataBool();

    // Get value
    bool GetValue() const;

    // Set value
    void SetValue(bool val);

    // Save / Load ...
    void Load(MemFile* File);
    void Write(MemFile* File) const;

    void        SetName(char const* szName);
    char const* GetName() const;
};
