#pragma once

#include <string>
#include <unordered_map>

#include <libs/math/c_vector.h>
#include <libs/renderer/dx9render.h>
#include <libs/util/probability_table.hpp>
#include <storm_audio/device.h>

#include "sound_defines.h"
#include "v_sound_service.h"

class INIFILE;

class SoundService: public VSoundService
{
public:
    struct Alias {
        float min_distance;
        float max_distance;
        float volume;

        storm::ProbabilityTable<std::string> sound_files;
    };

    struct PlayingSound {
        std::shared_ptr<storm::audio::Source> source;

        VolumeType volume_type {VolumeType::Fx};
        SoundType  sound_type {};
        float      volume {1.0F};

        // temp
        std::string name;

        uint16_t stamp {0};
        bool     is_free {true};
    };

    struct SoundSchemeChannel {
        std::string name;
        int32_t     min_delay_time;
        int32_t     max_delay_time;
        float       volume;
        int32_t     time_to_next_play;
        bool        is_looped;  // not working
    };

    struct CacheEntry {
        SoundType                            sound_type;
        std::shared_ptr<storm::audio::Sound> sound;
    };

    SoundService();
    ~SoundService() override;
    bool Init() override;

    uint32_t RunSection() override
    {
        return SECTION_EXECUTE;
    }

    void RunStart() override;
    void RunEnd() override;

    SoundID play(
        std::string const& name,
        SoundType          sound_type,
        VolumeType         volume_type,
        bool               is_paused      = false,
        bool               is_looped      = false,
        int32_t            fade_time      = 0,
        const CVECTOR*     start_position = nullptr,
        float              min_distance   = -1.0F,
        float              max_distance   = -1.0F,
        float              volume         = 1.0F) override;

    SoundID  duplicate(SoundID id) override;
    void     set_3d_param(SoundID id, SoundMessageType msg, void const* data) override;
    void     stop(SoundID id, int32_t fade_time = 0) override;
    void     sound_release(SoundID id) override;
    void     set_volume(SoundID id, float volume) override;
    bool     is_playing(SoundID id) override;
    uint32_t get_position(SoundID id) override;
    void     sound_restart(SoundID id) override;
    void     resume(SoundID id, int32_t fade_time = 0) override;

    // Service functions
    void  set_master_volume(float fx_volume, float music_volume, float speech_volume) override;
    void  get_master_volume(float& fx_volume, float& music_volume, float& speech_volume) override;
    void  set_pitch(float pitch) override;
    float get_pitch() override;
    void  set_camera_position(const CVECTOR& camera_position) override;
    void  set_camera_orientation(const CVECTOR& nose, const CVECTOR& head) override;

    // Schemes routines
    void reset_scheme() override;
    bool set_scheme(std::string_view const& scheme_name) override;
    bool add_scheme(std::string_view const& scheme_name) override;
    void set_enabled(bool is_enabled) override;

    void set_active_with_fade(bool is_active) override;

private:
    SoundID prepare_music(std::string const& name, int32_t fade_time = 0);

    SoundID prepare_sound(
        std::string const& name,
        SoundType          sound_type,
        const CVECTOR*     start_position = nullptr,
        float              min_distance   = -1.0F,
        float              max_distance   = -1.0F);

    void free_playing_on_source(std::shared_ptr<storm::audio::Source> const& source);
    void update_playing_list();

    void set_volume_all(float volume);
    void set_volume_for_sound(PlayingSound& sound, float volume);

    void resume_all(int32_t fade_time = 0);
    void resume_sound(PlayingSound& sound, int32_t fade_time = 0);

    void stop_all(int32_t fade_time = 0);
    void stop_sound(PlayingSound& sound, int32_t fade_time = 0);

    bool is_id_valid(SoundID id);

    float get_volume_by_type(PlayingSound const& sound) const;

    std::shared_ptr<storm::audio::Sound> get_from_cache(std::string const& sound_path, SoundType sound_type);

    // Aliases ------------------------------------------------------------

    void add_alias(INIFILE& ini_file, std::string_view const& section_name);
    void load_alias_file(std::string const& filename) override;
    void init_aliases();

    // Sound Schemes------------------------------------------------------------

    bool add_sound_scheme_channel(std::string const& in_string, bool is_looped = false);
    void process_sound_schemes();
    bool allocate_sound(SoundID& id);

    std::unique_ptr<storm::audio::Device> m_device;

    VDX9RENDER* m_renderer;

    bool m_is_initialized;
    bool m_fader_parity;

    std::vector<PlayingSound>       m_playing_sounds;
    std::vector<SoundSchemeChannel> m_sound_scheme_channels;

    std::unordered_map<std::string, Alias>                     m_aliases;
    std::unordered_map<std::string, std::chrono::milliseconds> m_ogg_pos;

    std::unordered_multimap<std::string, CacheEntry> m_sound_cache;

    float m_fx_volume;
    float m_music_volume;
    float m_speech_volume;

    float m_pitch;

    std::chrono::milliseconds m_fade_time;
};
