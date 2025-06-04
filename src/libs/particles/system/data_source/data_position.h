#pragma once

#include <string>

#include <libs/math/math3d.h>
#include <libs/particles/i_common/mem_file.h>

class DataPosition
{
    std::string Name;

    Vector Value;

public:
    // constructor / destructor
    DataPosition();
    virtual ~DataPosition();

    // Get value (Current time, Random factor [0..1])
    Vector const& GetValue() const;

    // Set value
    void SetValue(Vector const& val);

    void Load(MemFile* File);
    void Write(MemFile* File) const;

    void        SetName(char const* szName);
    char const* GetName() const;
};
