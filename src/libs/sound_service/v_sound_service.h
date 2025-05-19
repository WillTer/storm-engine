#pragma once

#include <limits>

#include <libs/core/service.h>
#include <libs/math/c_vector.h>

#include "sound_defines.h"

///////////////////////////////////////////////////////////////////
// DEFINES & TYPES
///////////////////////////////////////////////////////////////////

class TSD_ID
{
public:
    TSD_ID() : m_id(std::numeric_limits<uint32_t>::max()) {}

    TSD_ID(uint32_t id) : m_id(id) {}

    TSD_ID(int32_t id) : m_id(id) {}

    bool master() const
    {
        return m_id == 0;
    }

    bool is_valid() const
    {
        return m_id != std::numeric_limits<uint32_t>::max();
    }

    uint16_t index() const
    {
        return (m_id & std::numeric_limits<uint16_t>::max()) - 1;
    }

    uint16_t stamp() const
    {
        return (m_id >> 16) & std::numeric_limits<uint16_t>::max();
    }

    static TSD_ID createId(uint16_t index)
    {
        static uint16_t stamp_counter;
        return (index + 1) | (static_cast<uint32_t>(++stamp_counter) << 16);
    }

    operator int32_t() const
    {
        return m_id;
    }

private:
    uint32_t m_id;
};

using tSoundStatistics = struct {
    int32_t soundsCount, maxSoundsCount, bytesInBuffers, maxBytesInBuffers, bytesCached, maxBytesCached, totalMem, freeMem;
};

///////////////////////////////////////////////////////////////////
// INTERFACE DEFINITION
///////////////////////////////////////////////////////////////////
class VSoundService: public SERVICE
{
public:
    bool     Init() override       = 0;
    uint32_t RunSection() override = 0;
    void     RunStart() override   = 0;

    // Sound entries functions
    //
    // + play returns id of new loaded sound
    // + in most functions _id = 0 equals "apply to all"
    // + play assumes _name a plain file name
    // + for now loop_pause_time works only for OGG-STEREO sounds
    ///////////////////////////////////////////////////////////////
    virtual TSD_ID SoundPlay(
        char const*    _name,
        eSoundType     _type,                   // sound type
        eVolumeType    _volumeType,             // volume type
        bool           _simpleCache   = false,  // cache only, not play
        bool           _looped        = false,  // looped?
        bool           _cached        = false,  // unload after stoppping?
        int32_t        _time          = 0,      // fade in, if _time > 0
        const CVECTOR* _startPosition = nullptr,
        float          _minDistance   = -1.0f,
        float          _maxDistance   = -1.0f,
        int32_t        _loopPauseTime = 0,
        float          _volume        = 1.0f,
        int32_t        _prior         = 128) = 0;

    virtual TSD_ID   SoundDuplicate(TSD_ID _sourceID)                                     = 0;
    virtual void     SoundSet3DParam(TSD_ID _id, eSoundMessage _message, void const* _op) = 0;
    virtual void     SoundStop(TSD_ID _id, int32_t _time = 0)                             = 0;
    virtual void     SoundRelease(TSD_ID _id)                                             = 0;
    virtual void     SoundSetVolume(TSD_ID _id, float _volume)                            = 0;
    virtual bool     SoundIsPlaying(TSD_ID _id)                                           = 0;
    virtual uint32_t SoundGetPosition(TSD_ID _id)                                         = 0;
    virtual void     SoundRestart(TSD_ID _id)                                             = 0;
    virtual void     SoundResume(TSD_ID _id, int32_t _time = 0)                           = 0;

    // Service functions
    virtual void  SetMasterVolume(float _fxVolume, float _musicVolume, float _speechVolume)    = 0;
    virtual void  GetMasterVolume(float* _fxVolume, float* _musicVolume, float* _speechVolume) = 0;
    virtual void  SetPitch(float _pitch)                                                       = 0;
    virtual float GetPitch()                                                                   = 0;
    virtual void  SetCameraPosition(const CVECTOR& _cameraPosition)                            = 0;
    virtual void  SetCameraOrientation(const CVECTOR& _nose, const CVECTOR& _head)             = 0;

    virtual void ResetScheme()                      = 0;
    virtual bool SetScheme(char const* _schemeName) = 0;
    virtual bool AddScheme(char const* _schemeName) = 0;

    virtual void SetEnabled(bool _enabled)            = 0;
    virtual void LoadAliasFile(char const* _filename) = 0;

    virtual void SetActiveWithFade(bool active) = 0;

    tSoundStatistics soundStatistics;
};

/*
API_SERVICE_START("sound service")
    DECLARE_MAIN_SERVICE(SoundService)
API_SERVICE_END(SoundService)
*/
