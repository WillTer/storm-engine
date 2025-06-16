#include "sink_effect.h"

#include <libs/core/core.h>
#include <libs/core/entity.h>
#include <libs/filesystem/default_paths.h>
#include <libs/shared_headers/messages.h>
#include <libs/ship/ship_base.h>

//--------------------------------------------------------------------
SinkEffect::SinkEffect() : renderer(nullptr), sea(nullptr) {}

//--------------------------------------------------------------------
SinkEffect::~SinkEffect()
{
    // GUARD(SINKEFFECT::~SINKEFFECT)

    // UNGUARD
}

//--------------------------------------------------------------------
bool SinkEffect::Init()
{
    sea = static_cast<SEA_BASE*>(core->GetEntityPointer(core->GetEntityId("Sea")));

    renderer = static_cast<VDX9RENDER*>(core->GetService("RendererService"));

    InitializeSinks();

    return true;
    // UNGUARD
}

//--------------------------------------------------------------------
uint64_t SinkEffect::ProcessMessage(MESSAGE& message)
{
    // GUARD(SINKEFFECT::ProcessMessage)

    auto const     code     = message.Long();
    uint32_t const outValue = 0;

    switch (code) {
    case MSG_SHIP_DELETE: {
        auto* const attrs = message.AttributePointer();
        if (attrs) {
            auto&& entities = core->GetEntityIds("Ship");
            for (auto ent: entities) {
                /*
                shipBase = (SHIP_BASE *) core->GetEntityPointer(shipID);
                if (shipBase->GetACharacter() == attrs)
                {
                  TryToAddSink(shipBase->GetPos(), shipBase->GetBoxsize().z / 2.0f);
                  return outValue;
                }*/

                auto* shipBase = static_cast<SHIP_BASE*>(core->GetEntityPointer(ent));
                if (shipBase->GetACharacter() == attrs) {
                    TryToAddSink(shipBase->GetPos(), shipBase->GetBoxsize().z / 2.0f);
                    return outValue;
                }
            }  // if (FindClass)
        }  // if (attrs)
    }  // case
    break;
    }

    return outValue;
    // UNGUARD
}

//--------------------------------------------------------------------
void SinkEffect::Realize(uint32_t _dTime)
{
    // GUARD(SINKEFFECT::Realize)

    for (auto i = 0; i < sink_effect::MAX_SINKS; ++i)
        sinks[i].Realize(_dTime);

    // UNGUARD
}

//--------------------------------------------------------------------
void SinkEffect::Execute(uint32_t _dTime)
{
    // GUARD(SINKEFFECT::Execute)
    /*
      if (core->Controls->GetAsyncKeyState('X'))
      {
        if (renderer && sea)
        {
          static CVECTOR pos, ang, nose, head;
          static CMatrix view;
          //CVECTOR dir(randCentered(2.0f), -1.0f, randCentered(2.0f));
          CVECTOR dir(0.0f, -1.0f, 0.0f);

          renderer->GetTransform(D3DTS_VIEW, view);
          view.Transposition();
          nose = view.Vz();
          //head = view.Vy();
          pos = view.Pos();

          pos += 10.0f * !nose;
          pos.y = sea->WaveXZ(pos.x, pos.y);
          TSink *sinks= TryToAddSink(pos, 10.0f);
        }
      }
    */
    for (auto i = 0; i < sink_effect::MAX_SINKS; ++i)
        sinks[i].Process(_dTime);

    // UNGUARD
}

//--------------------------------------------------------------------
void SinkEffect::InitializeSinks()
{
    // FIXME: hardcode
    auto psIni = fio->open_ini_file(fio->base_directory_path(BaseDirectory::Ini) / "particles.ini");

    for (auto i = 0; i < sink_effect::MAX_SINKS; ++i) {
        sinks[i].Release();
        sinks[i].Initialize(psIni.get(), nullptr, sea, renderer);
    }
}

//--------------------------------------------------------------------
TSink* SinkEffect::TryToAddSink(const CVECTOR& _pos, float _r)
{
    for (auto i = 0; i < sink_effect::MAX_SINKS; ++i) {
        if (!sinks[i].Enabled()) {
            sinks[i].Start(_pos, _r);
            return &sinks[i];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------
