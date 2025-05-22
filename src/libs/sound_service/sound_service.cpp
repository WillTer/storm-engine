#include "sound_service.h"

#include <algorithm>
#include <thread>

// #include <fmod_errors.h>

#include <libs/core/core.h>
#include <libs/core/v_file_service.h>
#include <libs/core/vma.hpp>
#include <libs/math/math3d/color.h>
#include <libs/math/math_inlines.h>
#include <libs/math/matrix.h>
#include <libs/util/rands.h>

// for debugging
#include "openal/al_backend.h"

#include "debug_entity.h"

using namespace storm::audio;

CREATE_SERVICE(SoundService)

CREATE_CLASS(SoundVisualisationEntity)

#define DISTANCEFACTOR 1.0F

#define CHECK_RESULT(expr) error_handler(expr, __FILE__, __LINE__, __func__, #expr)

namespace
{

constexpr float VOLUME_DEFAULT = 0.5F;
constexpr float PITCH_DEFAULT  = 1.0F;
constexpr float FADE_DEFAULT   = 0.5F;

Result error_handler(Result const result, char const* file, unsigned line, char const* func, char const* expr)
{
    if (result != Result::Ok) { core.Trace("[%s:%d][%s][%s] Call error, code: %d", file, line, func, expr, static_cast<int>(result)); }
    return result;
}

void analyse_name_string_and_add_to_alias(SoundService::tAlias& alias, std::string_view const& name)
{
    static char file_name[COMMON_STRING_LENGTH];
    strncpy_s(file_name, name.data(), COMMON_STRING_LENGTH);

    float probability = DEFAULT_PROBABILITY;

    char* col = strchr(file_name, ',');
    if (col != nullptr) {
        // try to convert forthcoming number
        float flt = 0.0F;
        if (sscanf(++col, "%f", &flt) == 1) { probability = flt; }

        *(--col) = '\0';  // truncate at first ','
    }

    if constexpr (TRACE_INFORMATION) { core.Trace("  -> sound %s, %f", file_name, probability); }

    alias.soundFiles.emplace(probability, file_name);
}

}  // namespace

SoundService::SoundService()
{
    m_renderer = nullptr;

    fFXVolume     = VOLUME_DEFAULT;
    fMusicVolume  = VOLUME_DEFAULT;
    fSpeechVolume = VOLUME_DEFAULT;

    fPitch = PITCH_DEFAULT;

    bShowDebugInfo = false;
    initialized    = false;
    m_backend      = nullptr;

    listenerPos = {0.0F, 0.0F, 0.0F};
    listenerVel = {0.0F, 0.0F, 0.0F};
    listenerOri = {0.0F, 0.0F, 0.0F};

    m_fader_parity = false;
}

SoundService::~SoundService()
{
    if (initialized) {
        for (auto& music: m_music_sounds) {
            if (music) { music.reset(); }
        }
        for (auto& cache: SoundCache) {
            if (cache.sound) { cache.sound.reset(); }
        }
        m_backend.reset();
    }
}

bool SoundService::Init()
{
    initialized = false;

    m_renderer = static_cast<VDX9RENDER*>(core.GetService("DX9RENDER"));

    if (m_renderer == nullptr) { return false; }

    m_backend = std::make_unique<ALBackend>();
    if (!m_backend || CHECK_RESULT(m_backend->init()) != Result::Ok) { return false; }

    if (auto const ini = fio->OpenIniFile(core.EngineIniFileName())) {
        fadeTimeInSeconds = ini->GetFloat("sound", "fade_time", FADE_DEFAULT);
    }

    numActiveSounds = 2;  // 0 and 1 are reserved for music

    InitAliases();
    CreateEntityIfNeed();

    initialized = true;

    return true;
}

void SoundService::RunEnd()
{
    CreateEntityIfNeed();

    m_backend->update();
}

// TODO: move to backend
void SoundService::ProcessFader(uint16_t const idx)
{
    if (PlayingSounds[idx].bFree) { return; }

    auto const delta_time_s = core.GetDeltaTime() * 0.001F;

    PlayingSounds[idx].fFaderCurrentVolume += PlayingSounds[idx].fFaderDeltaInSec * delta_time_s;

    // FADE IN
    if (PlayingSounds[idx].fFaderDeltaInSec > 0) {
        PlayingSounds[idx].fFaderCurrentVolume = std::min(PlayingSounds[idx].fFaderCurrentVolume, PlayingSounds[idx].fFaderNeedVolume);
    }

    auto const channel_lock = PlayingSounds[idx].channel.lock();
    if (!channel_lock) { return; }

    // FADE OUT
    if (PlayingSounds[idx].fFaderDeltaInSec < 0) {
        if (fabsf(PlayingSounds[idx].fFaderCurrentVolume - PlayingSounds[idx].fFaderNeedVolume) < std::numeric_limits<float>::epsilon()) {
            PlayingSounds[idx].fFaderCurrentVolume = PlayingSounds[idx].fFaderNeedVolume;
            std::chrono::milliseconds ogg_pos      = {};
            CHECK_RESULT(channel_lock->get_playback_position(ogg_pos));
            SetOGGPosition(PlayingSounds[idx].Name.c_str(), ogg_pos);
            CHECK_RESULT(channel_lock->stop());
        }
    }

    CHECK_RESULT(channel_lock->set_volume(PlayingSounds[idx].fFaderCurrentVolume));
}

uint16_t SoundService::FreeSound(uint16_t const idx)
{
    PlayingSounds[idx].bFree = true;
    if (idx >= 2 && idx < numActiveSounds) { freeSounds.push(idx); }

    return idx;
}

void SoundService::RunStart()
{
    CreateEntityIfNeed();

    if (m_renderer) {
        static CVECTOR pos, nose, head;
        static CMatrix view;

        m_renderer->GetTransform(D3DTS_VIEW, view);
        view.Transposition();
        nose = view.Vz();
        head = view.Vy();
        pos  = view.Pos();

        SetCameraPosition(pos);
        SetCameraOrientation(nose, head);
    }

    ProcessFader(0);
    ProcessFader(1);

    auto const release_sound = [this](uint16_t const idx) {
        if (idx <= 1 && m_music_sounds[idx]) { m_music_sounds[idx].reset(); }

        FreeSound(idx);
        core.Event("SoundEnded", "l", idx + 2);
    };

    // release the sounds that have played
    for (uint16_t i = 0; i < numActiveSounds; i++) {
        if (PlayingSounds[i].bFree) { continue; }

        auto const channel_lock = PlayingSounds[i].channel.lock();
        if (!channel_lock) {
            release_sound(i);
            continue;
        }

        ChannelState state = {};
        CHECK_RESULT(channel_lock->get_state(state));

        if (state == ChannelState::Stopped) { release_sound(i); }
    }

    ProcessSoundSchemes();
}

bool SoundService::AllocateSound(TSD_ID& id)
{
    if (!freeSounds.empty()) {
        id = TSD_ID::createId(freeSounds.top());
        freeSounds.pop();
        return true;
    }

    if (numActiveSounds > MAX_SOUNDS_SLOTS) {
        core.Trace("SoundService::AllocateSound(): no empty slots!");
        return false;
    }

    id = TSD_ID::createId(numActiveSounds++);
    return true;
}

std::string SoundService::get_random_name(tAlias const& alias) const
{
    return alias.soundFiles.pickRandom();
}

size_t SoundService::get_alias_index_by_name(std::string_view const& name) const
{
    uint32_t const dwSearchHash = MakeHashValue(name.data());
    for (size_t i = 0; i < Aliases.size(); i++) {
        if (Aliases[i].dwNameHash == dwSearchHash && Aliases[i].Name == name) { return i; }
    }
    return std::numeric_limits<size_t>::max();
}

TSD_ID SoundService::SoundPlay(
    char const*    _name,
    eSoundType     _type,
    eVolumeType    _volumeType,
    bool           _simpleCache /* = false*/,
    bool           _looped /* = false*/,
    bool           _cached /* = false*/,
    int32_t        _time /* = 0*/,
    const CVECTOR* _startPosition /* = 0*/,
    float          _minDistance /* = -1.0f*/,
    float          _maxDistance /* = -1.0f*/,
    int32_t        _loopPauseTime /* = 0*/,
    float          _volume, /* = 1.0f*/
    int32_t        _prior)
{
    std::string file_name = _name;

    // aliases don`t contain `\`
    if (strchr(_name, '\\') == nullptr) {
        // Trying to find in aliases
        auto const alias_idx = get_alias_index_by_name(_name);
        if (alias_idx != std::numeric_limits<decltype(alias_idx)>::max() && !Aliases[alias_idx].soundFiles.empty()) {
            // play sound from the alias ...
            file_name = get_random_name(Aliases[alias_idx]);
            if constexpr (TRACE_INFORMATION) { core.Trace("Play sound from alias %s", file_name.c_str()); }

            _minDistance = Aliases[alias_idx].fMinDistance;
            _maxDistance = Aliases[alias_idx].fMaxDistance;
            _prior       = Aliases[alias_idx].iPrior;

            if (Aliases[alias_idx].fVolume > std::numeric_limits<float>::epsilon()) { _volume = Aliases[alias_idx].fVolume; }
        }
    }

    std::string sound_name = "resource\\sounds\\" + file_name;
    sound_name             = fio->ConvertPathResource(sound_name.c_str());

    std::shared_ptr<ISound> sound     = nullptr;
    uint16_t                sound_idx = 0;

    TSD_ID id;
    if (_type == MP3_STEREO) {
        if (m_backend->create_sound(sound_name, ISound::Flags::Stream | ISound::Flags::Stereo2D, sound) != Result::Ok) {
            core.Trace("Error creating sound stream for file %s\n", sound_name.c_str());
            return 0;
        }

        auto music_idx     = static_cast<uint16_t>(m_fader_parity);
        m_fader_parity     = !m_fader_parity;
        auto old_music_idx = static_cast<uint16_t>(m_fader_parity);

        if (m_music_sounds[old_music_idx]) { SoundStop(old_music_idx + 1, _time); }

        sound_idx = music_idx;
        id        = sound_idx + 1;

        m_music_sounds[sound_idx]                    = sound;
        PlayingSounds[sound_idx].fFaderNeedVolume    = _volume * fMusicVolume;
        PlayingSounds[sound_idx].fFaderCurrentVolume = 0.0F;
        PlayingSounds[sound_idx].fFaderDeltaInSec    = _volume * fMusicVolume;
        if (_time != 0) { PlayingSounds[sound_idx].fFaderDeltaInSec /= (_time * 0.001F); }
    } else {
        // For all other sounds, take from the cache
        auto const cache_idx = GetFromCache(sound_name, _type);
        if (cache_idx == std::numeric_limits<decltype(cache_idx)>::max()) {
            core.Trace("No sound \"%s\" in cache", sound_name.c_str());
            return 0;
        }

        if (!AllocateSound(id)) { return 0; }

        sound_idx                      = id.index();
        PlayingSounds[sound_idx].stamp = id.stamp();

        sound = SoundCache[cache_idx].sound;
    }

    //--------
    PlayingSounds[sound_idx].type         = _volumeType;
    PlayingSounds[sound_idx].fSoundVolume = _volume;

    // Get channel for sound but do not start to play
    CHECK_RESULT(m_backend->bind_sound_to_empty_channel(sound, PlayingSounds[sound_idx].channel));

    auto const channel_lock = PlayingSounds[sound_idx].channel.lock();
    if (!channel_lock) { return 0; }

    if (sound_idx <= 1) {
        auto const music_pos = GetOGGPosition(sound_name.c_str());
        channel_lock->set_playback_position(music_pos);

        _prior = 0;
    }

    _prior = std::clamp(_prior, 0, 255);

    // put priority ...
    // PlayingSounds[sound_idx].channel->setPriority(_prior);

    // Adjust parameters for 3D channel ...
    if (_type == PCM_3D) {
        channel_lock->set_min_distance(std::max(_minDistance, 0.0F) * DISTANCEFACTOR);
        channel_lock->set_max_distance(std::max(_maxDistance, 0.0F) * DISTANCEFACTOR);

        std::array<float, 3> position = {};
        if (_startPosition != nullptr) {
            position[0] = _startPosition->x;
            position[1] = _startPosition->y;
            position[2] = _startPosition->z;
        }

        channel_lock->set_position_3d(position);
    }

    switch (_volumeType) {
    case VOLUME_FX: _volume *= fFXVolume; break;
    case VOLUME_MUSIC: _volume *= fMusicVolume; break;
    case VOLUME_SPEECH: _volume *= fSpeechVolume; break;
    default: _volume *= 1.0F; break;
    }

    if (_time <= 0) {
        channel_lock->set_volume(_volume);
    } else {
        channel_lock->set_volume(0);
    }

    channel_lock->set_pitch(fPitch);

    PlayingSounds[sound_idx].Name       = std::move(sound_name);
    PlayingSounds[sound_idx].sound_type = _type;

    if (!_simpleCache) {
        // If we're not caching just start to play
        channel_lock->play();
    }

    PlayingSounds[sound_idx].bFree = false;
    channel_lock->set_looping(_looped);

    // ---------- loop through all sounds looking for the one with the same channel --------------
    for (uint16_t j = 0; j < numActiveSounds; j++) {
        if (j == sound_idx) { continue; }

        if (PlayingSounds[j].channel.lock() == channel_lock) {
            // note that the sound is thrown out ...
            // so as not to stop him ...
            j = FreeSound(j);
            break;
        }
    }

    return id;
}

void SoundService::SoundSet3DParam(TSD_ID id, eSoundMessage message_type, void const* data)
{
    if (id.master() || id.index() > numActiveSounds) return;

    auto& sound = PlayingSounds[id.index()];

    if (id.stamp() != sound.stamp) return;

    auto const channel_lock = sound.channel.lock();
    if (!channel_lock) { return; }

    switch (message_type) {
    case SM_MAX_DISTANCE: {
        float const distance = *reinterpret_cast<float const*>(data);
        CHECK_RESULT(channel_lock->set_max_distance(distance));
    } break;

    case SM_MIN_DISTANCE: {
        float const distance = *reinterpret_cast<float const*>(data);
        CHECK_RESULT(channel_lock->set_min_distance(distance));
    } break;

    case SM_POSITION: {
        auto        pos   = std::array<float, 3> {};
        auto const* array = reinterpret_cast<float const*>(data);
        std::memcpy(pos.data(), array, sizeof(pos));
        CHECK_RESULT(channel_lock->set_position_3d(pos));
    } break;
    }
}

void SoundService::SoundRestart(TSD_ID id)
{
    if constexpr (TRACE_INFORMATION) core.Trace("Sound restart !");
}

void SoundService::SoundRelease(TSD_ID id)
{
    if constexpr (TRACE_INFORMATION) core.Trace("Sound release !");
}

void SoundService::SoundSetVolume(TSD_ID id, float const volume)
{
    if constexpr (TRACE_INFORMATION) core.Trace("Sound set volume !");

    // TODO: separate function
    if (id.master()) {
        for (uint16_t i = 0; i < numActiveSounds; i++) {
            if (PlayingSounds[i].bFree) continue;

            PlayingSounds[i].fSoundVolume = volume;

            float actual_volume = volume;
            switch (PlayingSounds[i].type) {
            case VOLUME_FX: actual_volume *= fFXVolume; break;
            case VOLUME_MUSIC: actual_volume *= fMusicVolume; break;
            case VOLUME_SPEECH: actual_volume *= fSpeechVolume; break;
            default: break;
            }

            if (i <= 1) {
                PlayingSounds[i].fFaderNeedVolume    = actual_volume;
                PlayingSounds[i].fFaderCurrentVolume = PlayingSounds[i].fFaderNeedVolume;
            }

            auto const channel_lock = PlayingSounds[i].channel.lock();
            if (!channel_lock) { continue; }
            CHECK_RESULT(channel_lock->set_volume(actual_volume));
        }
        return;
    }

    if (id.index() >= numActiveSounds) return;

    auto& sound = PlayingSounds[id.index()];
    if (id.stamp() != sound.stamp) return;

    if (id.index() <= 1 && (sound.fFaderCurrentVolume - sound.fFaderNeedVolume) > std::numeric_limits<float>::epsilon()) { return; }

    sound.fSoundVolume = volume;

    float actual_volume = volume;
    switch (sound.type) {
    case VOLUME_FX: actual_volume *= fFXVolume; break;
    case VOLUME_MUSIC: actual_volume *= fMusicVolume; break;
    case VOLUME_SPEECH: actual_volume *= fSpeechVolume; break;
    default: break;
    }

    auto const channel_lock = sound.channel.lock();
    if (!channel_lock) { return; }
    CHECK_RESULT(channel_lock->set_volume(actual_volume));
}

bool SoundService::SoundIsPlaying(TSD_ID id)
{
    if (id.master() || id.index() >= numActiveSounds) return false;

    auto& sound = PlayingSounds[id.index()];

    if (id.stamp() != sound.stamp) return false;

    return !sound.bFree;
}

void SoundService::SoundResume(TSD_ID id, int32_t time /* = 0*/)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Resume sound %d", id.index()); }

    if (id.master() || id.index() == 0) {
        for (uint16_t i = 0; i < numActiveSounds; i++) {
            if (PlayingSounds[i].bFree) { continue; }

            auto const channel_lock = PlayingSounds[i].channel.lock();
            if (!channel_lock) { continue; }
            CHECK_RESULT(channel_lock->play());
        }
        return;
    }

    if (id.index() >= numActiveSounds) return;

    auto& sound = PlayingSounds[id.index()];

    if (id.stamp() != sound.stamp) return;

    if (id.index() <= 1) {
        auto const channel_lock = sound.channel.lock();
        if (!channel_lock) { return; }
        CHECK_RESULT(channel_lock->play());
    }
}

uint32_t SoundService::SoundGetPosition(TSD_ID id)
{
    if (id.index() >= numActiveSounds) return 0;

    auto& sound = PlayingSounds[id.index()];

    if (id.stamp() != sound.stamp) return 0;

    auto const channel_lock = sound.channel.lock();
    if (!channel_lock) { return 0; }

    std::chrono::milliseconds pos = {};
    CHECK_RESULT(channel_lock->get_playback_position(pos));
    return static_cast<uint32_t>(pos.count());
}

void SoundService::SetCameraPosition(const CVECTOR& camera_pos)
{
    CHECK_RESULT(m_backend->set_listener_position_3d(std::array<float, 3> {camera_pos.x, camera_pos.y, camera_pos.z}));
}

void SoundService::SetCameraOrientation(const CVECTOR& nose, const CVECTOR& head)
{
    auto const nose_normalized = !nose;
    auto const head_normalized = !head;

    CHECK_RESULT(m_backend->set_listener_orientation_3d(std::array<float, 6> {
        nose_normalized.x, nose_normalized.y, nose_normalized.z, head_normalized.x, head_normalized.y, head_normalized.z}));
}

void SoundService::SetMasterVolume(float fx_volume, float music_volume, float speech_volume)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Set master volume"); }

    fFXVolume     = std::clamp(fx_volume, 0.0F, 1.0F);
    fMusicVolume  = std::clamp(music_volume, 0.0F, 1.0F);
    fSpeechVolume = std::clamp(speech_volume, 0.0F, 1.0F);

    for (uint16_t i = 0; i < numActiveSounds; ++i) {
        if (PlayingSounds[i].bFree) { continue; }

        float actual_volume = PlayingSounds[i].fSoundVolume;

        switch (PlayingSounds[i].type) {
        case VOLUME_FX: actual_volume *= fFXVolume; break;
        case VOLUME_MUSIC: actual_volume *= fMusicVolume; break;
        case VOLUME_SPEECH: actual_volume *= fSpeechVolume; break;
        default: break;
        }

        if (auto const channel_lock = PlayingSounds[i].channel.lock();
            !channel_lock || CHECK_RESULT(channel_lock->set_volume(actual_volume)) != Result::Ok) {
            i = FreeSound(i);
        }
    }
}

void SoundService::GetMasterVolume(float* fx_volume, float* music_volume, float* speech_volume)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Get master volume"); }

    *fx_volume     = fFXVolume;
    *music_volume  = fMusicVolume;
    *speech_volume = fSpeechVolume;
}

void SoundService::SetPitch(float pitch)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Set pitch"); }

    fPitch = pitch > std::numeric_limits<float>::epsilon() ? pitch : 0.0F;

    for (uint16_t i = 0; i < numActiveSounds; i++) {
        if (PlayingSounds[i].bFree) continue;

        if (auto const channel_lock = PlayingSounds[i].channel.lock();
            !channel_lock || CHECK_RESULT(channel_lock->set_pitch(fPitch)) != Result::Ok) {
            i = FreeSound(i);
        }
    }
}

float SoundService::GetPitch()
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Get pitch"); }

    return fPitch;
}

TSD_ID SoundService::SoundDuplicate(TSD_ID source_id)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Sound duplicate"); }

    return 0;
}

void SoundService::SetEnabled(bool enabled) {}

void SoundService::SetActiveWithFade(bool const active)
{
    if (fadeTimeInSeconds == 0.0F) { return; }

    if (m_backend == nullptr) { return; }

    // if (active) { system->mixerResume(); }

    // FMOD::ChannelGroup* mastergroup;
    // CHECKFMODERR(system->getMasterChannelGroup(&mastergroup));
    // unsigned long long parentclock;  // type should match with getDSPClock param from fmod.hpp
    // int                rate;
    // system->getSoftwareFormat(&rate, nullptr, nullptr);
    // mastergroup->getDSPClock(nullptr, &parentclock);

    // auto const dsp_clock_start = parentclock;
    // auto const dsp_clock_end   = static_cast<unsigned long long>(parentclock + fadeTimeInSeconds * rate);

    // if (active) { mastergroup->setDelay(dsp_clock_start, 0, false); }

    for (auto const& sound: PlayingSounds) {
        if (sound.bFree) { continue; }

        auto const channel_lock = sound.channel.lock();
        if (!channel_lock) { continue; }

        // TODO
        if (active) {
            channel_lock->play();
        } else {
            channel_lock->pause();
        }

        // if (active)
        // {
        //     PlayingSound.channel->addFadePoint(dsp_clock_start, 0.0f);
        //     PlayingSound.channel->addFadePoint(dsp_clock_end, 1.0f);
        // }
        // else
        // {
        //     PlayingSound.channel->addFadePoint(dsp_clock_start, 1.0f);
        //     PlayingSound.channel->addFadePoint(dsp_clock_end, 0.0f);
        // }
    }

    // if (!active)
    // {
    //     mastergroup->setDelay(dsp_clock_start, dsp_clock_end, false);
    //     system->mixerSuspend();
    // }
}

void SoundService::SoundStop(TSD_ID id, int32_t time)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Stop sound %d", id.index()); }

    // TODO: separate method
    if (id.master()) {
        // --------- remove all sounds -----------------------------------------
        int start = 0;
        for (; time > 0 && start < 2; ++start) {
            if (PlayingSounds[start].bFree) { continue; }

            auto const channel_lock = PlayingSounds[start].channel.lock();
            if (!channel_lock) { continue; }

            float vol = 0.0F;
            CHECK_RESULT(channel_lock->get_volume(vol));
            PlayingSounds[start].fFaderNeedVolume    = 0.0F;
            PlayingSounds[start].fFaderCurrentVolume = vol;
            PlayingSounds[start].fFaderDeltaInSec    = -vol;
            if (time != 0) { PlayingSounds[start].fFaderDeltaInSec /= (time * 0.001F); }

            if (PlayingSounds[start].fFaderDeltaInSec < std::numeric_limits<float>::epsilon() && m_music_sounds[start]) {
                m_music_sounds[start].reset();
            }
        }

        for (uint16_t i = start; i < numActiveSounds; i++) {
            if (PlayingSounds[i].bFree) { continue; }

            auto const channel_lock = PlayingSounds[i].channel.lock();
            if (!channel_lock) {
                i = FreeSound(i);
                continue;
            }

            if (i <= 1) {
                std::chrono::milliseconds music_pos = {};
                CHECK_RESULT(channel_lock->get_playback_position(music_pos));
                SetOGGPosition(PlayingSounds[i].Name.c_str(), music_pos);
            }

            ChannelState state = ChannelState::None;
            CHECK_RESULT(channel_lock->get_state(state));
            if (state != ChannelState::Playing) {
                if constexpr (TRACE_INFORMATION) {
                    core.Trace(
                        "PlayingSounds[%d].channel 0x%08X %s state %d",
                        i,
                        channel_lock,
                        PlayingSounds[i].Name.c_str(),
                        static_cast<int>(state));
                }

                i = FreeSound(i);
            } else {
                CHECK_RESULT(channel_lock->stop());
            }
        }

        if (time <= 0) {
            m_music_sounds[0].reset();
            m_music_sounds[1].reset();
        }
        // --------- remove all sounds -----------------------------------------
        return;
    }

    // Delete the selected
    if (id.index() >= numActiveSounds) { return; }

    auto& sound = PlayingSounds[id.index()];

    auto const channel_lock = sound.channel.lock();

    if (id.stamp() != sound.stamp || !channel_lock) { return; }

    if (time > 0) {
        float vol = 0.0F;
        CHECK_RESULT(channel_lock->get_volume(vol));
        sound.fFaderNeedVolume    = 0.0F;
        sound.fFaderCurrentVolume = vol;
        sound.fFaderDeltaInSec    = -vol;
        if (time != 0) { sound.fFaderDeltaInSec /= (time * 0.001F); }
    } else {
        if (id.index() <= 1) {
            std::chrono::milliseconds music_pos = {};
            CHECK_RESULT(channel_lock->get_playback_position(music_pos));
            SetOGGPosition(sound.Name.c_str(), music_pos);
        }

        ChannelState state = ChannelState::None;
        CHECK_RESULT(channel_lock->get_state(state));
        if (!sound.bFree) {
            if (state != ChannelState::Playing) {
                FreeSound(id.index());
            } else {
                CHECK_RESULT(channel_lock->stop());
            }
        }

        if (id.index() <= 1 && m_music_sounds[id.index()]) { m_music_sounds[id.index()].reset(); }
    }
}

void SoundService::AddAlias(INIFILE& ini_file, std::string_view const& section_name)
{
    if (section_name.empty()) { return; }

    static char temp_string[COMMON_STRING_LENGTH];

    if constexpr (TRACE_INFORMATION) { core.Trace("Add sound alias %s", section_name.data()); }

    Aliases.push_back(tAlias {});
    tAlias& alias      = Aliases.back();
    alias.Name         = section_name;
    alias.dwNameHash   = MakeHashValue(alias.Name.c_str());
    alias.fMaxDistance = ini_file.GetFloat(section_name.data(), "maxDistance", -1.0F);
    alias.fMinDistance = ini_file.GetFloat(section_name.data(), "minDistance", -1.0F);
    alias.fVolume      = ini_file.GetFloat(section_name.data(), "volume", -1.0F);
    alias.iPrior       = ini_file.GetInt(section_name.data(), "prior", 128);

    if (ini_file.ReadString(section_name.data(), "name", temp_string, COMMON_STRING_LENGTH, "")) {
        analyse_name_string_and_add_to_alias(alias, temp_string);
        while (ini_file.ReadStringNext(section_name.data(), "name", temp_string, COMMON_STRING_LENGTH)) {
            analyse_name_string_and_add_to_alias(alias, temp_string);
        }
    }
}

void SoundService::LoadAliasFile(char const* filename)
{
    constexpr int const section_name_length = 128;
    static char         section_name[section_name_length];

    std::string ini_name = ALIAS_DIRECTORY;
    ini_name += filename;

    if constexpr (TRACE_INFORMATION) { core.Trace("Find sound alias file %s", ini_name.c_str()); }

    auto alias_ini = fio->OpenIniFile(ini_name.c_str());
    if (!alias_ini) { return; }

    if (alias_ini->GetSectionName(section_name, section_name_length)) {
        AddAlias(*alias_ini, section_name);
        while (alias_ini->GetSectionNameNext(section_name, section_name_length)) {
            AddAlias(*alias_ini, section_name);
        }
    }
}

void SoundService::InitAliases()
{
    auto const filenames = fio->_GetPathsOrFilenamesByMask(ALIAS_DIRECTORY, "*.ini", false);
    for (std::string cur_name: filenames) {
        LoadAliasFile(cur_name.c_str());
    }
}

void SoundService::CreateEntityIfNeed()
{
    auto debug_ent_id = core.GetEntityId("SoundVisualisationEntity");
    if (debug_ent_id == 0U) {
        debug_ent_id       = core.CreateEntity("SoundVisualisationEntity");
        auto* debug_entity = static_cast<SoundVisualisationEntity*>(core.GetEntityPointer(debug_ent_id));
        debug_entity->SetMasterSoundService(this);
        debug_entity->Wakeup();
    }
}

void SoundService::DebugDraw()
{
    if (core.Controls->GetDebugAsyncKeyState('J') < 0) {
        bShowDebugInfo = !bShowDebugInfo;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (!bShowDebugInfo) { return; }

    // bool bShowDistances = true;

    int Ypos      = 38 + 16;
    int Count     = 0;
    int RealCount = 0;

    // FMOD_VECTOR lpos, lvel, lforward, lup;
    // system->get3DListenerAttributes(0, &lpos, &lvel, &lforward, &lup);

    // auto vListener = CVECTOR(lpos.x, lpos.y, lpos.z);

    // FMOD_CPU_USAGE usage;
    // system->getCPUUsage(&usage);
    // float fTotal = usage.dsp + usage.stream + usage.geometry + usage.update + usage.convolution1 +
    // usage.convolution1;

    // int CurrentAlloc, PeakAlloc;
    // FMOD::Memory_GetStats(&CurrentAlloc, &PeakAlloc);

    // CVECTOR list_pos;
    // list_pos.x = lpos.x;
    // list_pos.y = lpos.y;
    // list_pos.z = lpos.z;
    // // rs->DrawSphere(list_pos, 4.0f, 0xFF008000);

    // rs->Print(0, 0, "CPU Usage %3.2f Mem: %3.2f Kb, MemPeak %3.2f Kb, Cached %d sounds", fTotal, CurrentAlloc /
    // 1024.0f,
    //           PeakAlloc / 1024.0f, SoundCache.size());
    // rs->Print(0, 16, "position  %3.2f, %3.2f, %3.2f, forward %3.2f, %3.2f, %3.2f, up %3.2f, %3.2f, %3.2f", lpos.x,
    //           lpos.y, lpos.z, lforward.x, lforward.y, lforward.z, lup.x, lup.y, lup.z);

    CMatrix ind;
    ind.SetIdentity();
    m_renderer->SetWorld(ind);

    for (uint16_t i = 0; i < numActiveSounds; i++) {
        if (PlayingSounds[i].bFree) { continue; }

        // bool bVirtual;
        // PlayingSounds[i].channel->isVirtual(&bVirtual);

        // bool paused;
        // PlayingSounds[i].channel->getPaused(&paused);

        // float fMin, fMax;
        // PlayingSounds[i].channel->get3DMinMaxDistance(&fMin, &fMax);

        // FMOD_MODE sound_mode;
        // PlayingSounds[i].channel->getMode(&sound_mode);
        // bool bIsLooped = false;
        // if ((sound_mode & FMOD_LOOP_NORMAL) != 0)
        //     bIsLooped = true;

        // float fVol;
        // PlayingSounds[i].channel->getVolume(&fVol);

        // bool bPlaying;
        // PlayingSounds[i].channel->isPlaying(&bPlaying);

        // unsigned int position;
        // PlayingSounds[i].channel->getPosition(&position, FMOD_TIMEUNIT_MS);

        // if (!bVirtual)
        RealCount++;
        Count++;

        // FMOD_VECTOR pos, vel;
        // PlayingSounds[i].channel->get3DAttributes(&pos, &vel);

        // float audib = 0;
        // PlayingSounds[i].channel->getAudibility(&audib);

        // int prior;
        // PlayingSounds[i].channel->getPriority(&prior);

        if (PlayingSounds[i].sound_type == PCM_3D) {
            // 0xFFFFFF00 plays but cannot be heard
            // 0xFFFF0000 not playing
            // 0xFF00FF00 is playing

            // auto drawColor = Color(0xFFFF0000);

            // if (!bVirtual)
            // {
            //     // morph between yellow audib 0 and green audib 1 colors
            //     auto Zero = Color(0xFFFFFF00);
            //     auto Full = Color(0xFF00FF00);
            //     drawColor.Lerp(Zero, Full, audib);
            // }

            // CVECTOR vec_pos;
            // vec_pos.x = pos.x;
            // vec_pos.y = pos.y;
            // vec_pos.z = pos.z;
            // rs->DrawSphere(vec_pos, 0.2f, drawColor.GetDword());

            // if (bShowDistances && audib > 0.001f)
            // {
            //     rs->SetWorld(ind);
            //     Draw2DCircle(vec_pos, drawColor.GetDword(), fMin, drawColor.GetDword(), fMax);
            // }

            // float fDistance = CVECTOR(vListener - vec_pos).GetLength();

            // DebugPrint3D(vec_pos, 30.0f, 2, 1.0f, drawColor.GetDword(), 1.0f, "%s", PlayingSounds[i].Name.c_str());
            // DebugPrint3D(vec_pos, 30.0f, 0, 1.0f, drawColor.GetDword(), 1.0f, "%3.2f m", fDistance);
            // DebugPrint3D(vec_pos, 30.0f, 4, 1.0f, drawColor.GetDword(), 1.0f, "prior: %d", prior);
        } else {
            // rs->Print(0, Ypos,
            //           "[%d] [vol:%f] [Pos:%d] [Loop:%d] [Play:%d] sound [Pause:%d][Vir:%d][Aud:%f] '%s' 0x%08X", i,
            //           fVol, position, bIsLooped, bPlaying, paused, bVirtual, audib, PlayingSounds[i].Name.c_str(),
            //           PlayingSounds[i].channel);
            Ypos += 16;
        }

        // rs->Print(0, Ypos, "pos %f, %f, %f", pos.x, pos.y, pos.z);
        // Ypos += 16;
        // rs->Print(0, Ypos, "min %f,  max %f", fMin, fMax);
        // Ypos += 16;

        // rs->DrawSphere(vec_pos, fMax, 0xFFFF0000);
    }

    m_renderer->Print(0, 32, "Real sounds %d, Total sounds %d", RealCount, Count);
    m_renderer->Print(800, 0, "Sound schemes %d", SoundSchemeChannels.size());
    Ypos = 16;
    for (size_t i = 0; i < SoundSchemeChannels.size(); i++) {
        m_renderer->Print(810, Ypos, "[%d] %s", i, SoundSchemeChannels[i].soundName.c_str());
        Ypos += 16;
    }
}

size_t SoundService::GetFromCache(std::string_view const& name, eSoundType sound_type)
{
    uint32_t const search_hash = MakeHashValue(name.data());

    for (size_t i = 0; i < SoundCache.size(); i++) {
        if (SoundCache[i].type != sound_type) { continue; }
        if (SoundCache[i].dwNameHash == search_hash && SoundCache[i].Name == name) {
            SoundCache[i].fTimeFromLastPlay = 0.0;
            return i;
        }
    }

    auto flags = ISound::Flags::None;
    if (sound_type == PCM_3D) { flags = flags | ISound::Flags::Spatial3D; }
    if (sound_type == PCM_STEREO) { flags = flags | ISound::Flags::Stereo2D; }

    tSoundCache cache_value;
    CHECK_RESULT(m_backend->create_sound(name, flags, cache_value.sound));

    if (cache_value.sound == nullptr) {
        core.Trace("Problem with sound loading !!! '%s'", name.data());
        return std::numeric_limits<size_t>::max();
    }

    cache_value.type              = sound_type;
    cache_value.Name              = name;
    cache_value.dwNameHash        = search_hash;
    cache_value.fTimeFromLastPlay = 0.0F;

    SoundCache.push_back(cache_value);

    return SoundCache.size() - 1;
}

// Write text
void SoundService::DebugPrint3D(
    const CVECTOR& pos3D, float rad, int32_t line, float alpha, uint32_t color, float scale, char const* format, ...) const
{
    static char buf[256];
    // print to the buffer
    va_list args;
    va_start(args, format);
    // int32_t len = vsnprintf(buf, sizeof(buf) - 1, format, args);
    va_end(args);
    buf[sizeof(buf) - 1] = 0;
    // Looking for a point position on the screen
    static CMatrix      mtx, view, prj;
    static D3DVIEWPORT9 vp;
    MTX_PRJ_VECTOR      vrt;
    m_renderer->GetTransform(D3DTS_VIEW, view);
    m_renderer->GetTransform(D3DTS_PROJECTION, prj);
    mtx.EqMultiply(view, prj);
    view.Transposition();
    float dist = ~(pos3D - view.Pos());
    if (dist >= rad * rad) return;
    float const d = view.Vz() | view.Pos();
    if ((pos3D | view.Vz()) < d) return;
    m_renderer->GetViewport(&vp);
    mtx.Projection((CVECTOR*)&pos3D, &vrt, 1, vp.Width * 0.5f, vp.Height * 0.5f, sizeof(CVECTOR), sizeof(MTX_PRJ_VECTOR));
    // Looking for a position
    int32_t const fh = m_renderer->CharHeight(FONT_DEFAULT) / 2;
    vrt.y -= (line + 0.5f) * fh;
    // Transparency
    float const kDist = 0.75f;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    if (dist > kDist * kDist * rad * rad) {
        dist = 1.0f - (sqrtf(dist) - kDist * rad) / (rad - kDist * rad);
        alpha *= dist;
    }
    if (alpha <= 0.0f) return;
    color = (static_cast<uint32_t>(alpha * 255.0f) << 24) | (color & 0xffffff);
    // print the text
    m_renderer->ExtPrint(
        FONT_DEFAULT,
        color,
        0x00000000,
        PR_ALIGN_CENTER,
        false,
        scale,
        0,
        0,
        static_cast<int32_t>(vrt.x),
        static_cast<int32_t>(vrt.y),
        buf);
}

void SoundService::Draw2DCircle(const CVECTOR& center, uint32_t dwColor, float fRadius, uint32_t dwColor2, float fRadius2) const
{
    float   fDelta = 0.2f;
    RS_LINE line[2];
    CVECTOR vStart;
    CVECTOR vEnd;
    CVECTOR vStartPoint;

    vStartPoint = CVECTOR(cosf(0) * fRadius, 0.0f, sinf(0) * fRadius);
    vStartPoint += center;
    for (float Angle = fDelta; Angle <= (PI * 2); Angle += fDelta) {
        auto vPoint = CVECTOR(cosf(Angle) * fRadius, 0.0f, sinf(Angle) * fRadius);
        vPoint += center;

        vStart = vStartPoint;
        vEnd   = vPoint;

        line[0].vPos    = vStart;
        line[0].dwColor = dwColor;
        line[1].vPos    = vEnd;
        line[1].dwColor = dwColor;
        m_renderer->DrawLines(line, 1, "Line");
        // m_renderer->DrawVector(vStart, vEnd, dwColor);
        vStartPoint = vPoint;
    }

    vStart = vStartPoint;
    vEnd   = CVECTOR(cosf(0) * fRadius, 0.0f, sinf(0) * fRadius);
    vEnd += center;

    line[0].vPos    = vStart;
    line[0].dwColor = dwColor;
    line[1].vPos    = vEnd;
    line[1].dwColor = dwColor;
    m_renderer->DrawLines(line, 1, "Line");

    vStartPoint = CVECTOR(cosf(0) * fRadius2, 0.0f, sinf(0) * fRadius2);
    vStartPoint += center;
    for (float Angle = fDelta; Angle <= (PI * 2); Angle += fDelta) {
        auto vPoint = CVECTOR(cosf(Angle) * fRadius2, 0.0f, sinf(Angle) * fRadius2);
        vPoint += center;

        vStart = vStartPoint;
        vEnd   = vPoint;

        line[0].vPos    = vStart;
        line[0].dwColor = dwColor2;
        line[1].vPos    = vEnd;
        line[1].dwColor = dwColor2;
        m_renderer->DrawLines(line, 1, "Line");
        // m_renderer->DrawVector(vStart, vEnd, dwColor);
        vStartPoint = vPoint;
    }

    vStart = vStartPoint;
    vEnd   = CVECTOR(cosf(0) * fRadius2, 0.0f, sinf(0) * fRadius2);
    vEnd += center;

    line[0].vPos    = vStart;
    line[0].dwColor = dwColor2;
    line[1].vPos    = vEnd;
    line[1].dwColor = dwColor2;
    m_renderer->DrawLines(line, 1, "Line");

    line[0].vPos    = CVECTOR(fRadius, 0.0f, 0.0f) + center;
    line[0].dwColor = dwColor;
    line[1].vPos    = CVECTOR(fRadius2, 0.0f, 0.0f) + center;
    line[1].dwColor = dwColor;
    m_renderer->DrawLines(line, 1, "Line");

    line[0].vPos    = CVECTOR(-fRadius, 0.0f, 0.0f) + center;
    line[0].dwColor = dwColor;
    line[1].vPos    = CVECTOR(-fRadius2, 0.0f, 0.0f) + center;
    line[1].dwColor = dwColor;
    m_renderer->DrawLines(line, 1, "Line");

    line[0].vPos    = CVECTOR(0.0f, 0.0f, fRadius) + center;
    line[0].dwColor = dwColor;
    line[1].vPos    = CVECTOR(0.0f, 0.0f, fRadius2) + center;
    line[1].dwColor = dwColor;
    m_renderer->DrawLines(line, 1, "Line");

    line[0].vPos    = CVECTOR(0.0f, 0.0f, -fRadius) + center;
    line[0].dwColor = dwColor;
    line[1].vPos    = CVECTOR(0.0f, 0.0f, -fRadius2) + center;
    line[1].dwColor = dwColor;
    m_renderer->DrawLines(line, 1, "Line");

    // m_renderer->DrawVector(vStart, vEnd, dwColor);
}

//--------------------------------------------------------------------
bool SoundService::SFLB_SetScheme(char const* _schemeName)
{
    ResetScheme();
    return AddScheme(_schemeName);
}

bool SoundService::SetScheme(char const* _schemeName)
{
    return SFLB_SetScheme(_schemeName);
}

//--------------------------------------------------------------------
void SoundService::ResetScheme()
{
    SoundSchemeChannels.clear();
}

//--------------------------------------------------------------------
bool SoundService::AddScheme(char const* _schemeName)
{
    static char tempString[COMMON_STRING_LENGTH];
    auto        ini = fio->OpenIniFile(SCHEME_INI_NAME);

    if (!ini) return false;

    if (ini->ReadString(const_cast<char*>(_schemeName), SCHEME_KEY_NAME, tempString, COMMON_STRING_LENGTH, "")) {
        AddSoundSchemeChannel(tempString);
        while (ini->ReadStringNext(const_cast<char*>(_schemeName), SCHEME_KEY_NAME, tempString, COMMON_STRING_LENGTH))
            AddSoundSchemeChannel(tempString);
    }

    if (ini->ReadString(const_cast<char*>(_schemeName), SCHEME_KEY_NAME_LOOP, tempString, COMMON_STRING_LENGTH, "")) {
        AddSoundSchemeChannel(tempString, true);
        while (ini->ReadStringNext(const_cast<char*>(_schemeName), SCHEME_KEY_NAME_LOOP, tempString, COMMON_STRING_LENGTH))
            AddSoundSchemeChannel(tempString, true);
    }

    return true;
}

bool SoundService::AddSoundSchemeChannel(char* in_string, bool _looped /*= false*/)
{
    static char tempString2[COMMON_STRING_LENGTH];
    strncpy_s(tempString2, in_string, COMMON_STRING_LENGTH);

    char* col = strchr(tempString2, ',');
    if (!col) {
        // only name, without delays
        tSoundSchemeChannel NewChannel;
        NewChannel.soundName      = tempString2;
        NewChannel.minDelayTime   = SCHEME_MIN_DELAY;
        NewChannel.maxDelayTime   = SCHEME_MAX_DELAY;
        NewChannel.timeToNextPlay = static_cast<int32_t>(rand(static_cast<float>(NewChannel.maxDelayTime - NewChannel.minDelayTime)));
        NewChannel.volume         = 1.f;
        NewChannel.looped         = _looped;
        if (_looped) NewChannel.timeToNextPlay = 0;

        SoundSchemeChannels.push_back(NewChannel);
        return true;
    }

    tSoundSchemeChannel NewChannel;

    // try to convert forthcoming numbers
    int       n1, n2;
    float     f1;
    int const numbersConverted = sscanf(++col, "%d, %d, %f", &n1, &n2, &f1);
    switch (numbersConverted) {
    case 1:
        NewChannel.minDelayTime = SCHEME_MIN_DELAY;
        NewChannel.maxDelayTime = n1 * 1000;
        NewChannel.volume       = 1.f;
        break;
    case 2:
        NewChannel.minDelayTime = n1 * 1000;
        NewChannel.maxDelayTime = n2 * 1000;
        NewChannel.volume       = 1.f;
        break;
    case 3:
        NewChannel.minDelayTime = n1 * 1000;
        NewChannel.maxDelayTime = n2 * 1000;
        NewChannel.volume       = f1;
        break;
    }
    *(--col)             = 0;  // truncate at first ','
    NewChannel.soundName = tempString2;
    NewChannel.looped    = _looped;

    if (_looped)
        NewChannel.timeToNextPlay = 0;
    else
        NewChannel.timeToNextPlay = static_cast<int32_t>(rand(static_cast<float>(NewChannel.maxDelayTime - NewChannel.minDelayTime)));

    SoundSchemeChannels.push_back(NewChannel);
    return true;
}

void SoundService::ProcessSoundSchemes()
{
    // handle schemes
    uint32_t const dTime = core.GetDeltaTime();

    for (size_t i = 0; i < SoundSchemeChannels.size(); i++) {
        if (SoundSchemeChannels[i].looped) {
            if (SoundSchemeChannels[i].timeToNextPlay) continue;
            SoundSchemeChannels[i].timeToNextPlay = -1;
            SoundPlay(
                SoundSchemeChannels[i].soundName.c_str(),
                PCM_STEREO,
                VOLUME_FX,
                false,
                true,
                false,
                0,
                nullptr,
                -1.f,
                -1.f,
                0,
                SoundSchemeChannels[i].volume);
        } else {
            if (SoundSchemeChannels[i].timeToNextPlay > 0) {
                SoundSchemeChannels[i].timeToNextPlay -= dTime;
            } else {
                SoundSchemeChannels[i].timeToNextPlay = SoundSchemeChannels[i].minDelayTime
                    + static_cast<int32_t>(rand(
                        static_cast<float>(SoundSchemeChannels[i].maxDelayTime - SoundSchemeChannels[i].minDelayTime)));
                SoundPlay(
                    SoundSchemeChannels[i].soundName.c_str(),
                    PCM_STEREO,
                    VOLUME_FX,
                    false,
                    false,
                    false,
                    0,
                    nullptr,
                    -1.f,
                    -1.f,
                    0,
                    SoundSchemeChannels[i].volume);
            }
        }  // looped
    }
}

int SoundService::GetOGGPositionIndex(char const* sz_name)
{
    uint32_t const dw_hash = MakeHashValue(sz_name);

    for (size_t i = 0; i < OGGPosition.size(); i++) {
        if (OGGPosition[i].dwHash == dw_hash) {
            if (OGGPosition[i].Name == sz_name) { return i; }
        }
    }
    return -1;
}

std::chrono::milliseconds SoundService::GetOGGPosition(char const* sz_name)
{
    int const idx = GetOGGPositionIndex(sz_name);
    if (idx >= 0) { return OGGPosition[idx].position; }
    return std::chrono::milliseconds(0);
}

void SoundService::SetOGGPosition(char const* sz_name, std::chrono::milliseconds const& pos)
{
    int const idx = GetOGGPositionIndex(sz_name);
    if (idx >= 0) {
        OGGPosition[idx].position = pos;
        return;
    }

    PlayedOGG ogg;
    ogg.dwHash   = MakeHashValue(sz_name);
    ogg.Name     = sz_name;
    ogg.position = pos;

    OGGPosition.push_back(ogg);
}
