#pragma once

#include <string>
#include <vector>

#include "../../i_common/graph_vertex.h"
#include "../../i_common/mem_file.h"

class DataGraph
{
    std::string Name;

    // when was the last time the value was read
    float MaxCachedTime;
    float MinCachedTime;
    // What was the index of this time
    uint32_t MaxCachedIndex;
    uint32_t MinCachedIndex;

    std::vector<GraphVertex> MinGraph;
    std::vector<GraphVertex> MaxGraph;

    void ResetCachedTime();

    float GetMinAtTime(float Time, float LifeTime);
    float GetMaxAtTime(float Time, float LifeTime);

    bool bRelative;
    bool bNegative;

public:
    // constructor / destructor
    DataGraph();
    virtual ~DataGraph();

    // Set / get can be negative values in the graph or not
    void SetNegative(bool _bNegative);
    bool GetNegative() const;

    // Set / get relative schedule or not
    void SetRelative(bool _bRelative);
    bool GetRelative() const;

    // Get value (Current time, Random factor [0..1])
    float GetValue(float Time, float LifeTime, float K_rand);
    float GetRandomValue(float Time, float LifeTime);

    // Set values
    void SetValues(GraphVertex const* MinValues, uint32_t MinValuesSize, GraphVertex const* MaxValues, uint32_t MaxValuesSize);

    // Set the "default"
    void SetDefaultValue(float MaxValue, float MinValue);

    // Get the count in the minimum graph
    uint32_t GetMinCount() const;

    // Get the count in the maximum graph
    uint32_t GetMaxCount() const;

    // Get the value by index from the minimum graph
    GraphVertex const& GetMinVertex(uint32_t Index);

    // Get the value by index from the maximum graph
    GraphVertex const& GetMaxVertex(uint32_t Index);

    void Load(MemFile* File);
    void Write(MemFile* File);

    void        SetName(char const* szName);
    char const* GetName() const;

    float GetMaxTime();

    void ConvertRadToDeg();
    void ConvertDegToRad();
    void MultiplyBy(float Val);
    void Clamp(float MinValue, float MaxValue);
    void Reverse();  // Graphs = 1.0f - Graph
    void NormalToPercent();
    void PercentToNormal();
    void NormalToAlpha();
    void AlphaToNormal();
};
