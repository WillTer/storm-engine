#include "sound.h"

#include <libs/core/core.h>
#include <libs/core/vma.hpp>
#include <libs/shared_headers/messages.h>

CREATE_CLASS(SOUND)

#define MSG_SOUND_ALIAS_ADD 77017  //"s"          alias_name

//--------------------------------------------------------------------
SOUND::SOUND() : soundService(nullptr), renderer(nullptr) {}

//--------------------------------------------------------------------
SOUND::~SOUND() {}

//--------------------------------------------------------------------
bool SOUND::Init(std::shared_ptr<storm::ServiceLocator> const& service_locator)
{
    Entity::Init(service_locator);
    // GUARD(SOUND::Init)

    soundService = static_cast<VSoundService*>(core.GetService("SoundService"));
    if (!soundService) core.Trace("!SOUND: Can`t create sound service");

    renderer = static_cast<VDX9RENDER*>(core.GetService("dx9render"));
    core.AddToLayer(REALIZE, GetId(), -1);

    return true;
    // UNGUARD
}

//--------------------------------------------------------------------
uint64_t SOUND::ProcessMessage(MESSAGE& message)
{
    ////GUARD(SOUND::ProcessMessage)

    if (!soundService) return 0;

    auto     code = message.Long();
    CVECTOR  vector, vector2;
    int32_t  temp, temp2, temp3, vt;
    int32_t  id, tempLong;
    float    minD, maxD;
    float    v1, v2, v3;
    float    pitch;
    float    volume;
    VDATA *  vd1, *vd2, *vd3;
    VDATA*   pd;
    uint32_t outValue = 0;

    switch (code) {
    case MSG_SOUND_SET_MASTER_VOLUME:
        v1 = message.Float();
        v2 = message.Float();
        v3 = message.Float();
        soundService->set_master_volume(v1, v2, v3);
        break;
    case MSG_SOUND_GET_MASTER_VOLUME:
        soundService->get_master_volume(v1, v2, v3);
        vd1 = message.ScriptVariablePointer();
        vd2 = message.ScriptVariablePointer();
        vd3 = message.ScriptVariablePointer();
        vd1->Set(v1);
        vd2->Set(v2);
        vd3->Set(v3);
        break;
    case MSG_SOUND_SET_PITCH:
        pitch = message.Float();
        soundService->set_pitch(pitch);
        break;
    case MSG_SOUND_GET_PITCH:
        pitch = soundService->get_pitch();
        pd    = message.ScriptVariablePointer();
        pd->Set(pitch);
        break;
    case MSG_SOUND_SET_CAMERA_POSITION:
        vector.x = message.Float();
        vector.y = message.Float();
        vector.z = message.Float();
        soundService->set_camera_position(vector);
        break;
    case MSG_SOUND_SET_CAMERA_ORIENTATION:
        vector.x  = message.Float();
        vector.y  = message.Float();
        vector.z  = message.Float();
        vector2.x = message.Float();
        vector2.y = message.Float();
        vector2.z = message.Float();
        soundService->set_camera_orientation(vector, vector2);
        break;
    case MSG_SOUND_PLAY: {
        std::string const& tempString = message.String();  // filename

        temp = message.Long();  // type
        // defaults
        vt       = static_cast<int>(VolumeType::Fx);  // volume type
        temp2    = 0;
        temp3    = 0;
        tempLong = 0;
        vector.x = 0;
        vector.y = 0;
        vector.z = 0;
        minD     = -1.0f;
        maxD     = -1.0f;
        volume   = 1.f;
        // try to read as many parameters   as we can
        if (message.GetCurrentFormatType()) vt = message.Long();        // volume_type
        if (message.GetCurrentFormatType()) temp2 = message.Long();     // simple_cache
        if (message.GetCurrentFormatType()) temp3 = message.Long();     // looped
        if (message.GetCurrentFormatType()) message.Long();             // FIXME: remove cached
        if (message.GetCurrentFormatType()) tempLong = message.Long();  // fade_in_time
        // boal fix 28.10.06
        if (temp == SOUND_MP3_STEREO) {
            if (temp3)  // stereo OGG, looped
            {
                if (message.GetCurrentFormatType()) message.Long();  //  FIXME: remove loopPauseTime
                if (message.GetCurrentFormatType()) volume = message.Float();
            }
        } else {
            if (message.GetCurrentFormatType()) vector.x = message.Float();
            if (message.GetCurrentFormatType()) vector.y = message.Float();
            if (message.GetCurrentFormatType()) vector.z = message.Float();
            if (message.GetCurrentFormatType()) minD = message.Float();
            if (message.GetCurrentFormatType()) maxD = message.Float();
        }

        outValue = static_cast<uint32_t>(soundService->play(
            tempString,
            static_cast<SoundType>(temp),
            static_cast<VolumeType>(vt),
            (temp2 != 0),
            (temp3 != 0),
            tempLong,
            &vector,
            minD,
            maxD,
            volume));

        break;
    }
    case MSG_SOUND_STOP:
        id       = message.Long();
        tempLong = message.Long();
        soundService->stop(id, tempLong);
        break;
    case MSG_SOUND_RELEASE:
        id = message.Long();
        soundService->sound_release(id);
        break;
    case MSG_SOUND_DUPLICATE:
        id       = message.Long();
        outValue = static_cast<uint32_t>(soundService->duplicate(id));
        break;
    case MSG_SOUND_SET_3D_PARAM:
        id       = message.Long();
        tempLong = message.Long();
        switch (tempLong) {
        case SOUND_PARAM_MAX_DISTANCE:
            vector.x = message.Float();
            soundService->set_3d_param(id, SoundMessageType::MaxDistance, &(vector.x));
            break;
        case SOUND_PARAM_MIN_DISTANCE:
            vector.x = message.Float();
            soundService->set_3d_param(id, SoundMessageType::MinDistance, &(vector.x));
            break;
        case SOUND_PARAM_POSITION:
            vector.x = message.Float();
            vector.y = message.Float();
            vector.z = message.Float();
            soundService->set_3d_param(id, SoundMessageType::Position, &vector);
            break;
        }
        break;
    case MSG_SOUND_SET_VOLUME:
        id       = message.Long();
        vector.x = message.Float();
        soundService->set_volume(id, vector.x);
        break;
    case MSG_SOUND_IS_PLAYING:
        id       = message.Long();
        outValue = static_cast<uint32_t>(soundService->is_playing(id));
        break;
    case MSG_SOUND_GET_POSITION:
        id       = message.Long();
        outValue = soundService->get_position(id);
        break;
    case MSG_SOUND_RESTART:
        id = message.Long();
        soundService->sound_restart(id);
        break;
    case MSG_SOUND_RESUME:
        id   = message.Long();
        temp = message.Long();
        soundService->resume(id, temp);
        break;
    case MSG_SOUND_SCHEME_RESET: soundService->reset_scheme(); break;
    case MSG_SOUND_SCHEME_SET: {
        std::string const& tempString = message.String();
        soundService->set_scheme(tempString.c_str());
        break;
    }
    case MSG_SOUND_SCHEME_ADD: {
        std::string const& tempString = message.String();
        soundService->add_scheme(tempString.c_str());
        break;
    }

    case MSG_SOUND_SET_ENABLED: soundService->set_enabled(message.Long() != 0); break;

    case MSG_SOUND_ALIAS_ADD: {
        std::string const& tempString = message.String();
        soundService->load_alias_file(tempString.c_str());
        break;
    }
    }

    return outValue;
    ////UNGUARD
}

//--------------------------------------------------------------------
void SOUND::Realize(uint32_t dTime)
{
    if (!soundService) return;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------
