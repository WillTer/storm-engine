#include "data_desc.h"

#include <libs/util/storm_assert.h>

DataDescripion::DataDescripion()
{
    ItemsCount = 0;
}

DataDescripion::~DataDescripion() {}

void DataDescripion::AddField(FieldType Type, char const* Name)
{
    Assert(ItemsCount < MAX_DESC_COUNT);
    Fields[ItemsCount].Type = Type;
    Fields[ItemsCount].Name = Name;
    ItemsCount++;
}

char const* DataDescripion::GetFieldName(uint32_t Index) const
{
    return Fields[Index].Name;
}

FieldType DataDescripion::GetFieldType(uint32_t Index) const
{
    return Fields[Index].Type;
}

int DataDescripion::GetFieldCount() const
{
    return ItemsCount;
}

void DataDescripion::Clear()
{
    ItemsCount = 0;
}
