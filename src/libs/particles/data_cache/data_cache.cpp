#include "data_cache.h"

#include <libs/core/core.h>
#include <libs/filesystem/default_paths.h>
#include <libs/filesystem/v_file_service.h>
#include <libs/util/string_compare.hpp>

bool ReadingAlreadyComplete;

// Constructor / destructor
DataCache::DataCache(IParticleManager* pManager)
{
    Master = pManager;
}

DataCache::~DataCache()
{
    ResetCache();
}

// Put data for the system in the cache
void DataCache::CacheSystem(char const* FileName)
{
    auto path    = fio->base_directory_path(BaseDirectory::Particles) / FileName;
    auto pathStr = path.extension().string();
    if (!storm::iEquals(pathStr, ".xps")) path += ".xps";
    pathStr = path.string();
    std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), tolower);
    // MessageBoxA(NULL, (LPCSTR)path.c_str(), "", MB_OK); //~!~

    auto sysFile = fio->open_file<std::ifstream>(pathStr, std::ios::binary);

    if (!sysFile.is_open()) {
        core->Trace("Particles: '%s' File not found !!!", pathStr.c_str());
        return;
    }

    auto const FileSize = fio->file_size(pathStr.c_str());

    auto* pMemBuffer = new uint8_t[FileSize];
    sysFile.read(reinterpret_cast<char*>(pMemBuffer), FileSize);

    // Create data from file ...
    CreateDataSource(pMemBuffer, FileSize, pathStr.c_str());

    delete[] pMemBuffer;
}

// Reset cache
void DataCache::ResetCache()
{
    for (auto n = 0; n < Cache.size(); n++) {
        if (Cache[n].pData) Cache[n].pData->Release();
    }

    Cache.clear();
}

// Get a pointer to data for a particle system
DataSource* DataCache::GetParticleSystemDataSource(char const* FileName)
{
    // std::string NameWithExt = FileName;
    // NameWithExt.AddExtention(".xps");
    // NameWithExt.Lower();
    std::filesystem::path path    = FileName;
    auto                  pathStr = path.extension().string();
    if (!storm::iEquals(pathStr, ".xps")) path += ".xps";
    pathStr = path.string();
    std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), tolower);

    for (auto n = 0; n < Cache.size(); n++) {
        if (Cache[n].FileName == pathStr) return Cache[n].pData;
    }

    return nullptr;
}

// Check pointer for validity
bool DataCache::ValidatePointer(DataSource* pData)
{
    for (auto n = 0; n < Cache.size(); n++)
        if (Cache[n].pData == pData)  // fix
            return true;

    return false;
}

void DataCache::CreateDataSource(void* pBuffer, uint32_t BufferSize, char const* SourceFileName)
{
    LoadedDataSource NewDataSource;
    NewDataSource.FileName = SourceFileName;
    NewDataSource.pData    = new DataSource(Master);
    Cache.push_back(NewDataSource);

    // core->Trace("\nCreate data source for file %s", SourceFileName);

    auto* ReadFile = new MemFile;
    ReadFile->OpenRead(pBuffer, BufferSize);
    NewDataSource.pData->Load(ReadFile);
    ReadFile->Close();
    delete ReadFile;
}

uint32_t DataCache::GetCachedCount() const
{
    return Cache.size();
}

char const* DataCache::GetCachedNameByIndex(uint32_t Index)
{
    return Cache[Index].FileName.c_str();
}
