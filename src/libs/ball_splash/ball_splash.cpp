#include "ball_splash.h"

#include <libs/core/core.h>
#include <libs/filesystem/default_paths.h>
#include <libs/filesystem/v_file_service.h>
#include <libs/renderer/dx9render.h>
#include <libs/shared_headers/messages.h>
#include <stdio.h>

//--------------------------------------------------------------------
BallSplash::BallSplash() : renderer(nullptr), sea(nullptr) {}

//--------------------------------------------------------------------
BallSplash::~BallSplash()
{
    // GUARD(BALLSPLASH::~BALLSPLASH)

    // UNGUARD
}

//--------------------------------------------------------------------
bool BallSplash::Init()
{
    sea = static_cast<SEA_BASE*>(core->GetEntityPointer(core->GetEntityId("Sea")));

    renderer = static_cast<VDX9RENDER*>(core->GetService("RendererService"));

    // core->CreateEntity(&arrowModel,"ModelR");
    // core->Send_Message(arrowModel,"ls",MSG_MODEL_LOAD_GEO, "fish01");
    InitializeSplashes();

    return true;
    // UNGUARD
}

//--------------------------------------------------------------------
uint64_t BallSplash::ProcessMessage(MESSAGE& message)
{
    // GUARD(BALLSPLASH::ProcessMessage)

    auto const     code     = message.Long();
    uint32_t const outValue = 0;

    switch (code) {
    case MSG_BALLSPLASH_ADD: {
        CVECTOR pos, dir;
        pos.x = message.Float();
        pos.y = message.Float();
        pos.z = message.Float();
        /*
        dir.x = message.Float();
        dir.y = message.Float();
        dir.z = message.Float();
        */

        dir.x = 0.0f;
        dir.y = -1.0f;
        dir.z = 0.0f;

        TryToAddSplash(pos, dir);
    } break;
    }

    return outValue;
    // UNGUARD
}

//--------------------------------------------------------------------
void BallSplash::Realize(uint32_t _dTime)
{
    // GUARD(BALLSPLASH::Realize)

    uint64_t ticks;
    RDTSC_B(ticks);

    TSplash::lockTicks    = 0;
    TSplash::fillTicks    = 0;
    TSplash::unlockTicks  = 0;
    TSplash::realizeTicks = 0;
    TSplash::processCount = 0;

    // draw bottom part
    TSplash::startRender = true;
    TSplash::topIndex    = 0;
    auto lastProcessed   = -1;
    for (auto i = 0; i < MAX_SPLASHES; ++i)
        if (splashes[i].Process(_dTime)) lastProcessed = i;
    if (lastProcessed != -1) splashes[lastProcessed].PostProcess();
    splashes[lastProcessed].Realize(_dTime);

    // draw top part
    auto const techniqueStarted = renderer->TechniqueExecuteStart("splash2");
    TSplash::startRender        = true;
    TSplash::topIndex           = 0;
    lastProcessed               = -1;
    for (auto i = 0; i < MAX_SPLASHES; ++i) {
        if (splashes[i].Process2(_dTime)) lastProcessed = i;
    }
    splashes[lastProcessed].PostProcess2();
    splashes[lastProcessed].Realize2(_dTime);
    if (techniqueStarted)
        while (renderer->TechniqueExecuteNext())
            ;

    RDTSC_E(ticks);
    /*
    if ((core->Controls->GetKeyState('Z') & 0x8000) != 0)
    {
      renderer->Print(0, 150, "splash: all = %d, count = %d", ticks/1000, TSplash::processCount);
      if (TSplash::processCount)
      {
        renderer->Print(0, 175, "splash: lock = %d()", TSplash::lockTicks / 1000);
        renderer->Print(0, 200, "splash: fill = %d(%d)", TSplash::fillTicks / 1000, TSplash::fillTicks /
    (TSplash::processCount*1000)); renderer->Print(0, 225, "splash: ulock = %d()", TSplash::unlockTicks / 1000);
        renderer->Print(0, 250, "splash: realize = %d(%d)", TSplash::realizeTicks / 1000, TSplash::realizeTicks /
    (2*1000));
      }
    }
    */
    // UNGUARD
}

//--------------------------------------------------------------------
void BallSplash::Execute(uint32_t dTime)
{
    // GUARD(BALLSPLASH::Execute)

    // UNGUARD
}

//--------------------------------------------------------------------
void BallSplash::InitializeSplashes()
{
    // FIXME: hardcode
    auto psIni = fio->open_ini_file(fio->base_directory_path(BaseDirectory::Ini) / "particles.ini");

    for (auto i = 0; i < MAX_SPLASHES; ++i) {
        splashes[i].Release();
        splashes[i].Initialize(psIni.get(), nullptr, sea, renderer);
    }
}

//--------------------------------------------------------------------
TSplash* BallSplash::TryToAddSplash(const CVECTOR& _pos, const CVECTOR& _dir)
{
    auto backDir = !_dir;
    backDir.y    = -backDir.y;

    for (auto i = 0; i < MAX_SPLASHES; ++i) {
        if (!splashes[i].Enabled()) {
            splashes[i].Start(_pos, backDir);
            return &splashes[i];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------
