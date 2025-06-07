//****************************************************************
//*
//*  Author : Sergey Makeev aka Joker, 2003
//*
//*
//****************************************************************

#pragma once

#include <string>
#include <vector>

#include "../particles.h"

class ParticleManager;

class ParticleService: public IParticleService
{
    IParticleManager* pDefaultManager;

    struct CreatedManager {
        ParticleManager* pManager;
        std::string      FileName;
        int              Line;
    };

    bool                        sysDelete;
    std::vector<CreatedManager> CreatedManagers;

public:
    // Constructor / destructor
    ParticleService();
    ~ParticleService() override;

    bool Init(std::shared_ptr<storm::ServiceLocator> const& service_locator) override;

    // Create a particle manager
    IParticleManager* CreateManagerEx(char const* ProjectName, char const* File, int Line) override;

    virtual void RemoveManagerFromList(IParticleManager* pManager);

    uint32_t          GetManagersCount() override;
    IParticleManager* GetManagerByIndex(uint32_t Index) override;

    IParticleManager* DefManager() override;
};
