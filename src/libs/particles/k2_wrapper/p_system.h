#pragma once

#include <libs/core/vma.hpp>
#include <libs/math/c_vector.h>
#include <libs/math/matrix.h>
#include <libs/particles/v_particle_system.h>

class IParticleSystem;
class Particles;

class PARTICLE_SYSTEM: public VPARTICLE_SYSTEM
{
    IParticleSystem* pSystem;
    Particles*       pManager;

public:
    PARTICLE_SYSTEM(IParticleSystem* _pSystem);
    ~PARTICLE_SYSTEM() override;
    void Stop() override;
    void SetEmitter(CVECTOR p, CVECTOR a) override;
    void LinkToObject(entid_t id, CVECTOR _LinkPos) override;
    void SetDelay(int32_t _delay) override;
    void SetLifeTime(uint32_t time) override;
    void StopEmitter() override;
    void Pause(bool _bPause) override;

public:
    void             SetManager(Particles* _pManager);
    IParticleSystem* GetSystem() const;
};
