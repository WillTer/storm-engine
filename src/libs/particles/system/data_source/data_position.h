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
    const Vector &GetValue() const;

    // Set value
    void SetValue(const Vector &val);

    void Load(MemFile *File);
    void Write(MemFile *File) const;

    void SetName(const char *szName);
    const char *GetName() const;
};
