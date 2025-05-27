#pragma once

#include <array>
#include <stack>
#include <string>

#include <libs/math/c_vector.h>
#include <libs/renderer/dx9render.h>
#include <libs/util/probability_table.hpp>
#include <storm_audio/backend.h>

#include "sound_defines.h"
#include "v_sound_service.h"

// #include <fmod.hpp>

#define MAX_SOUNDS_SLOTS 4095

class INIFILE;
// debug....
class SoundVisualisationEntity;

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////

class SoundService: public VSoundService
{
public:
    struct tAlias {
        std::string Name;
        uint32_t    dwNameHash;

        float                                fMinDistance;
        float                                fMaxDistance;
        int32_t                              iPrior;
        float                                fVolume;
        storm::ProbabilityTable<std::string> soundFiles;

        tAlias() {}
    };

private:
    VDX9RENDER* m_renderer;

    bool bShowDebugInfo;
    bool initialized;

    std::unique_ptr<storm::audio::Backend>              m_backend;
    std::array<std::shared_ptr<storm::audio::Sound>, 2> m_music_sounds;

    struct tSoundCache {
        uint32_t    dwNameHash;
        std::string Name;
        float       fTimeFromLastPlay;
        eSoundType  type;

        std::shared_ptr<storm::audio::Sound> sound;

        tSoundCache() : type()
        {
            dwNameHash        = 0;
            fTimeFromLastPlay = 0.0F;
            sound             = nullptr;
        }
    };

    struct tPlayedSound {
        float fFaderNeedVolume;
        float fFaderCurrentVolume;
        float fFaderDeltaInSec;

        std::shared_ptr<storm::audio::Source> source;

        eVolumeType type;
        eSoundType  sound_type;
        float       fSoundVolume;

        // temp
        std::string Name;

        uint16_t stamp;
        bool     bFree;

        tPlayedSound() : sound_type(), fSoundVolume(0)
        {
            // channel = nullptr;
            type = VOLUME_FX;

            fFaderNeedVolume    = 0;
            fFaderCurrentVolume = 0;
            fFaderDeltaInSec    = 0;

            stamp = 0;
            bFree = true;
        }
    };

    tPlayedSound         PlayingSounds[MAX_SOUNDS_SLOTS];
    std::stack<uint16_t> freeSounds;
    uint16_t             numActiveSounds {};

    struct PlayedOGG {
        std::string Name;
        uint32_t    dwHash;

        std::chrono::milliseconds position;
    };

    std::vector<PlayedOGG> OGGPosition;

    std::chrono::milliseconds GetOGGPosition(char const* sz_name);
    void                      SetOGGPosition(char const* sz_name, std::chrono::milliseconds const& pos);
    int                       GetOGGPositionIndex(char const* sz_name);

    std::vector<tSoundCache> SoundCache;

    size_t GetFromCache(std::string_view const& name, eSoundType sound_type);

    bool m_fader_parity;

    std::array<float, 3> listenerPos;  // Position
    std::array<float, 3> listenerVel;  // Velocity
    std::array<float, 6> listenerOri;  // Orientation

    void CreateEntityIfNeed();

    // Aliases ------------------------------------------------------------
    std::vector<tAlias> Aliases;

    std::string get_random_name(tAlias const& alias) const;
    size_t      get_alias_index_by_name(std::string_view const& name) const;
    void        AddAlias(INIFILE& ini_file, std::string_view const& section_name);
    void        LoadAliasFile(char const* _filename) override;
    void        InitAliases();

    // Sound Schemes------------------------------------------------------------
    struct tSoundSchemeChannel {
        TSD_ID      SoundID;
        std::string soundName;
        int32_t     minDelayTime;
        int32_t     maxDelayTime;
        float       volume;
        int32_t     timeToNextPlay;
        bool        looped;  // not working

        tSoundSchemeChannel()
        {
            SoundID = 0;
        }
    };

    std::vector<tSoundSchemeChannel> SoundSchemeChannels;

    bool AddSoundSchemeChannel(char* in_string, bool _looped = false);
    void ProcessSoundSchemes();

    //----------------------------------------------------------------------------

    bool AllocateSound(TSD_ID& id);

    float fFXVolume;
    float fMusicVolume;
    float fSpeechVolume;

    float fPitch;

    float fadeTimeInSeconds = 0.5f;

public:
    SoundService();
    ~SoundService() override;
    bool SFLB_SetScheme(char const* _schemeName);
    bool Init() override;

    uint32_t RunSection() override
    {
        return SECTION_EXECUTE;
    }

    void RunStart() override;
    void RunEnd() override;

    TSD_ID SoundPlay(
        char const*    _name,
        eSoundType     _type,
        eVolumeType    _volumeType,
        bool           _simpleCache   = false,
        bool           _looped        = false,
        bool           _cached        = false,
        int32_t        _time          = 0,
        const CVECTOR* _startPosition = nullptr,
        float          _minDistance   = -1.0f,
        float          _maxDistance   = -1.0f,
        int32_t        _loopPauseTime = 0,
        float          _volume        = 1.0f,
        int32_t        _prior         = 128) override;

    TSD_ID   SoundDuplicate(TSD_ID _sourceID) override;
    void     SoundSet3DParam(TSD_ID _id, eSoundMessage _message, void const* _op) override;
    void     SoundStop(TSD_ID _id, int32_t _time = 0) override;
    void     SoundRelease(TSD_ID _id) override;
    void     SoundSetVolume(TSD_ID _id, float _volume) override;
    bool     SoundIsPlaying(TSD_ID _id) override;
    uint32_t SoundGetPosition(TSD_ID _id) override;
    void     SoundRestart(TSD_ID _id) override;
    void     SoundResume(TSD_ID _id, int32_t _time = 0) override;

    // Service functions
    void  SetMasterVolume(float _fxVolume, float _musicVolume, float _speechVolume) override;
    void  GetMasterVolume(float* _fxVolume, float* _musicVolume, float* _speechVolume) override;
    void  SetPitch(float _pitch) override;
    float GetPitch() override;
    void  SetCameraPosition(const CVECTOR& _cameraPosition) override;
    void  SetCameraOrientation(const CVECTOR& _nose, const CVECTOR& _head) override;

    // Schemes routines
    void ResetScheme() override;
    bool SetScheme(char const* _schemeName) override;
    bool AddScheme(char const* _schemeName) override;
    void SetEnabled(bool _enabled) override;

    void SetActiveWithFade(bool active) override;

    // WTF is that?
    void DebugDraw();
    void
    DebugPrint3D(const CVECTOR& pos3D, float rad, int32_t line, float alpha, uint32_t color, float scale, char const* format, ...) const;
    void Draw2DCircle(const CVECTOR& center, uint32_t dwColor, float fRadius, uint32_t dwColor2, float fRadius2) const;

    void ProcessFader(uint16_t idx);

    uint16_t FreeSound(uint16_t idx);
};
