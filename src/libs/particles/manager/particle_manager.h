//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************

#pragma once

#include <string>
#include <vector>

#include "../i_particle_manager.h"
#include "../system/particle_processor/bb_processor.h"
#include "../system/particle_processor/mdl_processor.h"

class ParticleService;
class DataCache;
class GeomCache;
class ParticleSystem;
class DataSource;

// Particle Systems Manager
class ParticleManager: public IParticleManager
{
    struct CacheReloadedInfo {
        std::string Name;
        bool        AutoDeleted;
        Matrix      matWorld;
    };

    bool ShowStat;
    bool GlobalDelete;

    struct SystemDesc {
        ParticleSystem* pSystem;
    };

    std::vector<SystemDesc> Systems;

    // Particle Model Processor
    ModelProcessor* MDL_Processor;

    // Billboard Particle Processor
    BillBoardProcessor* BB_Processor;

    int32_t    pProjectTexture;
    int32_t    pProjectTextureNormalMap;
    DataCache* pDataCache;
    GeomCache* pGeomCache;

    std::string ShortProjectName;
    std::string TextureName;

    // The service that created the system
    ParticleService* pService;

    VDX9RENDER* pRS;

    void DeleteAllSystems();

    float    TimeFromLastStatUpdate;
    uint64_t nowTickTime;
    uint64_t nowUpdateTime;
    uint32_t ActiveSystems;
    uint32_t ActiveEmitters;
    uint32_t ActiveBillboardParticles;
    uint32_t ActiveModelParticles;

    std::vector<ParticleSystem*> DeleteQuery;

protected:
    ~ParticleManager() override;

    ParticleSystem* CreateParticleSystemFromDataSource(DataSource* pDataSource);

    uint32_t                 IteratorIndex;
    std::vector<std::string> EnumUsedGeom;

    bool FindInEnumUsedGeom(char const* GeomName);

public:
    // Create / delete
    ParticleManager(ParticleService* service);
    bool Release() override;

    // Get a pointer to Render / FileService
    VDX9RENDER* Render() override;

    // Open project
    bool OpenProject(char const* FileName) override;
    // Close project
    void CloseProject() override;

    // Remove from resource list (system)
    void RemoveResource(IParticleSystem* pResource) override;

    // Execute Particles
    void Execute(float DeltaTime) override;

    // Find out whether the system is available or not
    bool IsSystemAvailable(char const* FileName) override;

    // Get the global texture of the project
    int32_t GetProjectTexture() override;
    // Set project texture
    void SetProjectTexture(char const* FileName = nullptr) override;

    // Get project name
    char const* GetProjectFileName() override;

    // Create a particle system from a file (the file must be in the project)
    IParticleSystem* CreateParticleSystemEx(char const* FileName, char const* File, int Line) override;

    // Create an empty particle system, for the editor
    IParticleSystem* CreateEmptyParticleSystemEx(char const* FileName, int Line) override;

    BillBoardProcessor* GetBBProcessor() const;
    ModelProcessor*     GetMDLProcessor() const;

    bool ValidateSystem(IParticleSystem* pSystem) override;

    GEOS* GetModel(char const* FileName) const;

    uint32_t        GetCreatedSystemCount() const;
    ParticleSystem* GetCreatedSystemByIndex(uint32_t Index);

    bool ReadyForUse() override;

    void DefferedDelete(ParticleSystem* pSys);

    char const* GetProjectTextureName() override;

    void Editor_UpdateCachedData() override;

    virtual uint32_t    GetProjectSystemCount();
    virtual char const* GetProjectSystemName(uint32_t Index);

    char const* GetFirstGeomName(char const* FileName) override;
    char const* GetNextGeomName() override;

    virtual void CreateGeomCache();

    void         WriteSystemCache(char const* FileName) override;
    virtual void WriteSystemCache(char const* FileName, MemFile* pMemFile);
    virtual void WriteSystemCacheAs(char const* FileName, char const* NewName);

    virtual void LoadSystemCache(char const* FileName, MemFile* pMemFile);
    virtual void CacheReloaded();

    virtual FieldList* Editor_CreatePointEmitter(char const* SystemName, char const* EmitterName);
    virtual FieldList* Editor_CreateBillBoardParticle(char const* SystemName, char const* EmitterName, char const* ParticleName);
    virtual FieldList* Editor_CreateModelParticle(char const* SystemName, char const* EmitterName, char const* ParticleName);

    virtual void DeletePointEmitter(char const* SystemName, IEmitter* pEmitter);
    virtual void DeleteBillboard(char const* SystemName, IEmitter* pEmitter, FieldList* pParticles);
    virtual void DeleteModel(char const* SystemName, IEmitter* pEmitter, FieldList* pParticles);

    void OpenDefaultProject() override;
};
