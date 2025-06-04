#pragma once

#include <vector>

#include "../system/data_source/data_source.h"

class IParticleManager;

class DataCache
{
    IParticleManager* Master;

    struct LoadedDataSource {
        std::string FileName;
        DataSource* pData;

        LoadedDataSource()
        {
            pData = nullptr;
        }
    };

    std::vector<LoadedDataSource> Cache;

    void CreateDataSource(void* pBuffer, uint32_t BufferSize, char const* SourceFileName);

public:
    // Constructor / destructor
    DataCache(IParticleManager* pManager);
    ~DataCache();

    // Put data for the system in the cache
    void CacheSystem(char const* FileName);

    // Reset cache
    void ResetCache();

    // Get a pointer to data for a particle system
    DataSource* GetParticleSystemDataSource(char const* FileName);

    // Check pointer for validity
    bool ValidatePointer(DataSource* pData);

    uint32_t    GetCachedCount() const;
    char const* GetCachedNameByIndex(uint32_t Index);
};
