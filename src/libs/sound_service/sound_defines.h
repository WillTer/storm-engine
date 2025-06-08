#pragma once

#include <cstdint>

#include <libs/filesystem/default_paths.h>

// FIXME: hardcode
constexpr std::string_view SCHEME_INI_NAME = "sound_scheme.ini";

///////////////////////////////////////////////////////////////////
// CONSTANTS
///////////////////////////////////////////////////////////////////
#define TRACE_INFORMATION 0
#define FIO(a) fio->_##a
#define MAX_CACHED_DATA (4 * 1024 * 1024)

#define SOUND_INVALID_ID 0
#define MAX_SOUND_DUPLICATES 50
#define COMMON_STRING_LENGTH 2048
#define MAX_SOUND_DRIVERS 5
#define ERROR_POSITION -1.0f
#define MIN_VOLUME 0.01f

extern int32_t lg[100];

#define MAX_SOUNDS 150
#define MAX_QUEUE_SOUNDS 30
#define MAX_DISTANCE 500.0f     // distance beyond which 3D-sound isn`t created
#define MAX_NAMES_PER_ALIAS 50  // random-chosen names within alias
#define MAX_LOOP_PAUSE_TIME 0

#define LOOP_DISABLED -54321

#define SOUND_SECTION_NAME "sound"
#define SOUND_PATH_NAME "sound path"
#define SCHEME_KEY_NAME "ch"
#define SCHEME_KEY_NAME_LOOP "ch_loop"

#define DEFAULT_PROBABILITY 1.f
#define SCHEME_MAX_CHANNELS 100
#define SCHEME_MIN_DELAY 0
#define SCHEME_MAX_DELAY 0x7FFFFFFF

enum class SoundType {
    Sound3D     = 1,
    MusicStereo = 2,
    Music3D     = 3,
    SoundStereo = 4,
};

enum class VolumeType {
    Fx     = 1,
    Music  = 2,
    Speech = 3,
};

enum class SoundMessageType {
    MaxDistance = 1,
    MinDistance = 2,
    Position    = 3,
};

///////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////
#ifndef TRACE_INFORMATION
#define SOUND_TRACE(a) true
#define SOUND_TRACE2(a, b) true
#else
#define SOUND_TRACE(a) _VSYSTEM_API->Trace(a);
#define SOUND_TRACE2(a, b) _VSYSTEM_API->Trace(a, b);
#endif

#define THROW_STRING(s, p) \
    { \
        static char _tmpString[2048]; \
        sprintf_s(_tmpString, s, p); \
        throw _tmpString; \
    }

class SoundService;
extern SoundService* soundService;
