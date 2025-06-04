#pragma once

#include <string>
#include <vector>

#include <libs/math/math3d.h>

#include "../../i_common/mem_file.h"
#include "../../i_common/types.h"
#include "../data_desc/data_desc.h"

class DataColor;
class DataBool;
class DataFloat;
class DataGraph;
class DataString;
class DataPosition;
class DataUV;

class FieldList
{
public:
    struct FieldDesc {
        // This flag is used when converting
        bool MarkForDelete;

        // Name hash
        uint32_t HashValue;

        // A type
        FieldType Type;

        // Name
        std::string Name;

        // Pointer
        void* pPointer;
    };

private:
    std::vector<FieldDesc> Fields;

    void DeleteFieldData(FieldDesc const& pData);

    // =================== creating empty attributes
    void CreateEmptyBoolField(char const* Name, bool def_value);
    void CreateEmptyFloatField(char const* Name, float def_value);
    void CreateEmptyGraphField(char const* Name, float def_value_min, float def_value_max);
    void CreateEmptyPositionField(char const* Name, Vector const& def_value);
    void CreateEmptyStringField(char const* Name, char const* def_value);
    void CreateEmptyUVField(char const* Name);
    void CreateEmptyColorField(char const* Name, uint32_t def_value);

    // ==================== Loading attributes from a file
    void CreateBoolField(MemFile* pMemFile);
    void CreateFloatField(MemFile* pMemFile);
    void CreateGraphField(MemFile* pMemFile);
    void CreatePositionField(MemFile* pMemFile);
    void CreateStringField(MemFile* pMemFile);
    void CreateUVField(MemFile* pMemFile);
    void CreateColorField(MemFile* pMemFile);

    FieldDesc* FindField(char const* Name);

public:
    FieldList();
    ~FieldList();

    void DelAll();

    // Save / Load ...
    void Load(MemFile* File);
    void Write(MemFile* File);

    DataColor* FindColor(char const* AttrName);
    DataGraph* FindGraph(char const* AttrName);
    DataUV*    FindUV(char const* AttrName);

    DataBool*     FindBool(char const* AttrName);
    DataFloat*    FindFloat(char const* AttrName);
    DataString*   FindString(char const* AttrName);
    DataPosition* FindPosition(char const* AttrName);

    float         GetFloat(char const* AttrName, float def_value = 0.0f);
    int           GetFloatAsInt(char const* AttrName, int def_value = 0);
    bool          GetBool(char const* AttrName, bool def_value = false);
    char const*   GetString(char const* AttrName, char const* def_value = "");
    Vector const& GetPosition(char const* AttrName, Vector const& def_value = Vector(0.0f));

    float GetGraphVal(char const* AttrName, float Time, float LifeTime, float K_Rand, float def_value = 0.0f);
    float GetRandomGraphVal(char const* AttrName, float Time, float LifeTime, float def_value = 0.0f);

    uint32_t         GetFieldCount() const;
    FieldDesc const& GetFieldByIndex(uint32_t Index);

    // The conversion GUARANTEES that this DataSource will have the required fields
    // and there will be no unnecessary garbage ...
    void Convert(DataDescripion* pDataDescriptor);
};
