#pragma once

#include "../../i_common/types.h"
#include "../../i_particle_system.h"
#include "../../manager/particle_manager.h"
#include "../data_source/data_source.h"

class IEmitter;

class ParticleSystem: public IParticleSystem
{
    struct EmitterDesc {
        EmitterType Type;
        IEmitter*   pEmitter;
    };

    std::vector<EmitterDesc> Emitters;

    // Auto-delete system or regular
    bool AutoDeleted;

    // Do not emit new particles !!!
    bool EmissionPause;

    bool DeleteIfNeed();

    std::string SystemName;

protected:
    ~ParticleSystem() override;

    IEmitter* CreatePointEmitter(DataSource::EmitterDesc* pEmitter);

    void DeleteAllEmitters();

    ParticleManager* pMaster;

    Matrix matWorld;

public:
    // Create / delete
    ParticleSystem(ParticleManager* serv);
    bool Release() override;

    // Execute all particles
    uint32_t Execute(float DeltaTime) override;

    // Restart particle system
    void Restart(uint32_t RandomSeed) override;

    // Pause particle emission
    void PauseEmission(bool bPause) override;

    void Stop() override;
    // Find out whether emission is paused or not
    bool IsEmissionPaused() override;

    // Set whether auto-delete system or regular ...
    void AutoDelete(bool Enabled) override;
    // Find out if auto-delete system or not
    bool IsAutoDeleted() override;
    // Set transformation matrix for system
    void SetTransform(Matrix const& transform) override;
    void GetTransform(Matrix& _matWorld) override;

    void CreateFromDataSource(DataSource* pDataSource);

    ParticleManager* GetMaster() const;

    IEmitter* FindEmitter(char const* name);
    IEmitter* FindEmitterByData(FieldList* Data);

    void Teleport(Matrix const& transform) override;

    bool IsAlive() override;

    char const* GetName() override;
    void        SetName(char const* Name);

public:
    uint32_t    GetEmittersCount() const;
    IEmitter*   GetEmitterByIndex(uint32_t Index);
    EmitterType GetEmitterTypeByIndex(uint32_t Index);

    void Editor_UpdateCachedData();
};
