//============================================================================================
//    Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    Fader
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "fader.h"

#include <libs/asset_server/asset_server.h>
#include <libs/core/core.h>
#include <libs/core/entity.h>
#include <libs/renderer_next/fader_post_process.h>
#include <libs/renderer_next/i_renderer_next.h>
#include <libs/renderer_next/i_texture.h>
#include <libs/renderer_next/progress_image_scene.h>
#include <libs/shared_headers/messages.h>

// ============================================================================================
// Construction, destruction
// ============================================================================================

int32_t Fader::numberOfTips = 0;
int32_t Fader::currentTips  = -1;

Fader::Fader() : fadeIn(false), isStart(false), isAutodelete(false)
{
    eventStart = false;
    eventEnd   = false;
    deleteMe   = 0;

    m_fader_render = std::make_shared<storm::FaderPostProcess>();
}

Fader::~Fader() {}

// Initialization
bool Fader::Init()
{
    // check that it's the only one
    auto&& entities = core->GetEntityIds("Fader");
    for (auto eid: entities) {
        if (eid == GetId()) continue;

        if (fadeIn == static_cast<Fader*>(core->GetEntityPointer(eid))->fadeIn) {
            core->Trace("Fader::Init() -> Fader already created, %s", fadeIn ? "fade in phase" : "fade out phase");
        }
        //!!!
        // return false;
    }

    // Layers
    core->SetLayerType(FADER_REALIZE, layer_type_t::realize);
    core->AddToLayer(FADER_REALIZE, GetId(), -256);
    core->SetLayerType(FADER_EXECUTE, layer_type_t::execute);
    core->AddToLayer(FADER_EXECUTE, GetId(), -256);

    return true;
}

// Messages
uint64_t Fader::ProcessMessage(MESSAGE& message)
{
    auto const& asset_server   = core->get<storm::AssetServer>();
    auto const& renderer       = core->get<storm::RendererNext>();
    auto const& progress_image = core->get<storm::ProgressImageScene>();

    switch (message.Long()) {
    case FADER_OUT: {
        // Start screen dimming
        float fade_speed = message.Float();
        if (fade_speed > 0.0f) {
            fade_speed = 1.0f / fade_speed;
        } else {
            fade_speed = 0.0f;
        }
        m_fader_render->start_fade(1.0, -fade_speed);
        m_fader_render->set_next(progress_image->get_post_processor());
        progress_image->set_post_processor(m_fader_render);

        fadeIn       = false;
        isStart      = true;
        isAutodelete = message.Long() != 0;
        eventStart   = false;
        eventEnd     = false;
    } break;
    case FADER_IN: {
        // Start screen appearance
        float fade_speed = message.Float();
        if (fade_speed < 0.00001f) { fade_speed = 0.00001f; }
        fade_speed = 1.0f / fade_speed;
        m_fader_render->start_fade(0.0F, fade_speed);
        m_fader_render->set_next(progress_image->get_post_processor());
        progress_image->set_post_processor(m_fader_render);

        fadeIn       = true;
        isStart      = true;
        isAutodelete = message.Long() != 0;
        eventStart   = false;
        eventEnd     = false;
    } break;
    case FADER_STARTFRAME: break;
    case FADER_PICTURE: {
        std::string const& name = message.String();

        auto const texture = asset_server->load_texture_file(name);
        progress_image->set_picture(renderer->load_texture(texture));
    } break;
    case FADER_PICTURE0: {
        std::string const& name = message.String();

        auto const texture = asset_server->load_texture_file(name);
        progress_image->set_background(renderer->load_texture(texture));
    } break;
    default: break;
    }
    return 0;
}

// Work
void Fader::Execute(uint32_t delta_time)
{
    if (deleteMe) {
        ++deleteMe;
        if (deleteMe >= 3) core->EraseEntity(GetId());
    }
    if (eventStart) {
        eventStart = false;
        if (!fadeIn) {
            core->PostEvent("FaderEvent_StartFade", 0, "li", fadeIn, GetId());
        } else {
            core->PostEvent("FaderEvent_StartFadeIn", 0, "li", fadeIn, GetId());
        }
    }
    if (eventEnd) {
        eventEnd = false;
        deleteMe = isAutodelete;
        if (!fadeIn) {
            core->PostEvent("FaderEvent_EndFade", 0, "li", fadeIn, GetId());
        } else {
            core->PostEvent("FaderEvent_EndFadeIn", 0, "li", fadeIn, GetId());
        }
    }
}

void Fader::Realize(uint32_t delta_time)
{
    if (isStart) { eventStart = true; }

    m_fader_render->update(delta_time);
    eventEnd = m_fader_render->is_fade_finished();
    if (eventEnd) {
        auto const& progress_image = core->get<storm::ProgressImageScene>();
        progress_image->set_post_processor(m_fader_render->get_next());
        m_fader_render->set_next(nullptr);
    }

    isStart = false;
}
