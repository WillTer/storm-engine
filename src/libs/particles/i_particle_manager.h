//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************

#pragma once

#include <libs/renderer/dx9render.h>

class ParticleService;
class IParticleSystem;

#ifndef NULL
#define NULL 0L
#endif

#define CreateParticleSystem(name) CreateParticleSystemEx(name, __FILE__, __LINE__)

// Particle Systems Manager
class IParticleManager
{
protected:
    virtual ~IParticleManager() {};

public:
    // Create / delete
    IParticleManager(ParticleService* service) {};
    virtual bool Release() = 0;

    // Get a pointer to Render / FileService
    virtual VDX9RENDER* Render() = 0;

    // Open project
    virtual bool OpenProject(char const* FileName) = 0;
    // Close project
    virtual void CloseProject() = 0;

    // Remove from resource list (system)
    virtual void RemoveResource(IParticleSystem* pResource) = 0;

    // Execute Particles
    virtual void Execute(float DeltaTime) = 0;

    // Find out whether the system is available or not
    virtual bool IsSystemAvailable(char const* FileName) = 0;

    // Get the global texture of the project
    virtual int32_t GetProjectTexture() = 0;

    // Set project texture
    virtual void SetProjectTexture(char const* FileName = nullptr) = 0;

    // Get project name
    virtual char const* GetProjectFileName() = 0;

    // Create an empty particle system, for the editor ...
    virtual IParticleSystem* CreateEmptyParticleSystemEx(char const* FileName, int Line) = 0;

    // Create a particle system from a file (the file must be in the project !!!!!)
    virtual IParticleSystem* CreateParticleSystemEx(char const* FileName, char const* File, int Line) = 0;

    // Check whether the pointer to the particle system is valid, it might has already been deleted
    virtual bool ValidateSystem(IParticleSystem* pSystem) = 0;

    // Check if all particles have loaded asynchronously and if the manager is ready
    // to the creation of systems
    virtual bool ReadyForUse() = 0;

    virtual char const* GetProjectTextureName() = 0;

    virtual void Editor_UpdateCachedData() = 0;

    virtual char const* GetFirstGeomName(char const* FileName) = 0;
    virtual char const* GetNextGeomName()                      = 0;

    virtual void WriteSystemCache(char const* FileName) = 0;

    virtual void OpenDefaultProject() = 0;
};
