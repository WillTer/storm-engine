#include "sound_service.h"

#include <algorithm>

#include <libs/core/core.h>
#include <libs/core/v_file_service.h>
#include <libs/core/vma.hpp>
#include <libs/math/math3d/color.h>
#include <libs/math/math_inlines.h>
#include <libs/math/matrix.h>
#include <libs/util/rands.h>
#include <storm_audio/source.h>

using namespace storm::audio;

CREATE_SERVICE(SoundService)

#define DISTANCEFACTOR 1.0F

namespace
{

constexpr float VOLUME_DEFAULT = 0.5F;
constexpr float PITCH_DEFAULT  = 1.0F;
constexpr float FADE_DEFAULT   = 0.5F;

constexpr size_t STREAM_BUFFER_COUNT = 2;
constexpr size_t BUFFER_SAMPLE_COUNT = 16384;

void analyse_name_string_and_add_to_alias(SoundService::Alias& alias, std::string_view const& name)
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

    alias.sound_files.emplace(probability, file_name);
}

void free_sound(SoundService::PlayingSound& sound)
{
    sound.is_free = true;
    sound.source.reset();
}

class Tracer: public DebugTracer
{
    void trace_message(
        [[maybe_unused]] Severity    severity,
        std::string const&           message,
        std::filesystem::path const& source_file,
        size_t                       line,
        std::string const&           function_name) override
    {
        core.Trace("[%s:%zd][%s] %s", source_file.filename().string().c_str(), line, function_name.c_str(), message.c_str());
    }
};

}  // namespace

SoundService::SoundService()
{
    m_renderer = nullptr;

    m_fx_volume     = VOLUME_DEFAULT;
    m_music_volume  = VOLUME_DEFAULT;
    m_speech_volume = VOLUME_DEFAULT;

    m_pitch = PITCH_DEFAULT;

    m_is_initialized = false;
    m_device         = nullptr;

    m_fader_parity = false;
}

SoundService::~SoundService()
{
    if (m_is_initialized) {
        for (auto& sound: m_playing_sounds) {
            sound.source.reset();
        }

        m_sound_cache.clear();
    }
}

bool SoundService::Init()
{
    m_is_initialized = false;

    m_renderer = static_cast<VDX9RENDER*>(core.GetService("DX9RENDER"));

    if (m_renderer == nullptr) { return false; }

    m_device =
        std::make_unique<Device>(std::make_shared<Tracer>(), Device::DistanceModel::Inverse, STREAM_BUFFER_COUNT, BUFFER_SAMPLE_COUNT);
    if (!m_device) { return false; }

    constexpr float sec_to_ms_mult = 1000.0F;
    if (auto const ini = fio->OpenIniFile(core.EngineIniFileName())) {
        m_fade_time = std::chrono::milliseconds(static_cast<uint64_t>(ini->GetFloat("sound", "fade_time", FADE_DEFAULT) * sec_to_ms_mult));
    }

    // Reserve first two for music
    m_playing_sounds.resize(2);

    init_aliases();

    m_is_initialized = true;

    return true;
}

void SoundService::RunEnd()
{
    m_device->update(std::chrono::milliseconds(core.GetDeltaTime()));
}

void SoundService::RunStart()
{
    if (m_renderer != nullptr) {
        static CVECTOR pos  = {};
        static CVECTOR nose = {};
        static CVECTOR head = {};
        static CMatrix view = {};

        m_renderer->GetTransform(D3DTS_VIEW, view);
        view.Transposition();
        nose = view.Vz();
        head = view.Vy();
        pos  = view.Pos();

        set_camera_position(pos);
        set_camera_orientation(nose, head);
    }

    // release the sounds that have played
    for (auto& sound: m_playing_sounds) {
        if (sound.is_free) { continue; }
        if (sound.source->get_state() == SourceState::Free) { free_sound(sound); }
    }

    process_sound_schemes();
}

bool SoundService::allocate_sound(TSD_ID& id)
{
    auto free_sound = std::find_if(
        std::next(m_playing_sounds.begin(), 2), m_playing_sounds.end(), [](PlayingSound const& sound) { return sound.is_free; });
    if (free_sound != m_playing_sounds.end()) {
        id = TSD_ID::createId(static_cast<uint16_t>(std::distance(m_playing_sounds.begin(), free_sound)));
        return true;
    }

    if (m_playing_sounds.size() >= std::numeric_limits<uint16_t>::max()) {
        core.Trace("SoundService::allocate_sound(): no empty slots!");
        return false;
    }

    id = TSD_ID::createId(static_cast<uint16_t>(m_playing_sounds.size()));
    return true;
}

TSD_ID SoundService::play(
    std::string const& name,
    eSoundType         sound_type,
    eVolumeType        volume_type,
    bool               is_paused /* = false*/,
    bool               is_looped /* = false*/,
    int32_t            fade_time /* = 0*/,
    const CVECTOR*     start_position /* = 0*/,
    float              min_distance /* = -1.0f*/,
    float              max_distance /* = -1.0f*/,
    float              volume /* = 1.0f*/)
{
    auto file_name = name;

    // aliases don`t contain `\`
    if (name.find_first_of('\\') == std::string::npos && m_aliases.contains(name)) {
        auto& alias = m_aliases[name];

        // play sound from the alias ...
        file_name = alias.sound_files.pickRandom();
        if constexpr (TRACE_INFORMATION) { core.Trace("Play sound from alias %s", file_name.c_str()); }

        min_distance = alias.min_distance;
        max_distance = alias.max_distance;

        if (alias.volume > std::numeric_limits<float>::epsilon()) { volume = alias.volume; }
    }

    std::string sound_path = "resource\\sounds\\" + file_name;
    sound_path             = fio->ConvertPathResource(sound_path.c_str());

    uint16_t sound_idx = 0;

    TSD_ID id = 0;
    if (sound_type == MP3_STEREO) {
        auto const sound = m_device->create_sound_stream(sound_path, Sound::Flags::Stereo2D);
        if (!sound) {
            core.Trace("Error creating sound stream for file %s", sound_path.c_str());
            return 0;
        }

        auto music_idx     = static_cast<uint16_t>(m_fader_parity);
        m_fader_parity     = !m_fader_parity;
        auto old_music_idx = static_cast<uint16_t>(m_fader_parity);

        stop(old_music_idx + 1, fade_time);

        sound_idx = music_idx;
        id        = sound_idx + 1;

        m_playing_sounds[sound_idx].source = m_device->attach_sound_stream(sound);
    } else {
        auto const sound = get_from_cache(sound_path, sound_type);
        if (sound == nullptr) {
            core.Trace("Problem with sound loading !!! '%s'", sound_path.c_str());
            return 0;
        }

        if (!allocate_sound(id)) { return 0; }
        sound_idx = id.index();

        if (sound_idx == m_playing_sounds.size()) { m_playing_sounds.push_back({}); }

        auto& playing_sound  = m_playing_sounds[sound_idx];
        playing_sound.stamp  = id.stamp();
        playing_sound.source = m_device->attach_sound(sound);
    }

    if (!m_playing_sounds[sound_idx].source) { return 0; }

    //--------
    m_playing_sounds[sound_idx].volume_type = volume_type;
    m_playing_sounds[sound_idx].volume      = volume;

    if (sound_type == MP3_STEREO && fade_time > 0) {
        m_playing_sounds[sound_idx].source->set_volume_max(0.0F);
        m_playing_sounds[sound_idx].source->fade(
            0.0F, get_volume_by_type(m_playing_sounds[sound_idx]), std::chrono::milliseconds(fade_time));
    } else {
        m_playing_sounds[sound_idx].source->set_volume_max(get_volume_by_type(m_playing_sounds[sound_idx]));
    }

    if constexpr (TRACE_INFORMATION) {
        core.Trace(
            "Sound attached, name %s, idx = %d, channel = %p, state = %d",
            file_name.c_str(),
            sound_idx,
            m_playing_sounds[sound_idx].source.get(),
            m_playing_sounds[sound_idx].source->get_state());
    }

    if (sound_idx <= 1 && m_ogg_pos.contains(sound_path)) {
        m_playing_sounds[sound_idx].source->set_playback_position(m_ogg_pos.at(sound_path));
    }

    // Adjust parameters for 3D source
    if (sound_type == PCM_3D) {
        m_playing_sounds[sound_idx].source->set_min_distance(std::max(min_distance, 0.0F) * DISTANCEFACTOR);
        m_playing_sounds[sound_idx].source->set_max_distance(std::max(max_distance, 0.0F) * DISTANCEFACTOR);

        std::array<float, 3> position = {};
        if (start_position != nullptr) {
            position[0] = start_position->x;
            position[1] = start_position->y;
            position[2] = -start_position->z;
        }

        m_playing_sounds[sound_idx].source->set_position_3d(position);
    }

    m_playing_sounds[sound_idx].source->set_volume(1.0F);  // Volume level is multiplicated before calculating attenuation
    m_playing_sounds[sound_idx].source->set_volume_min(0.0F);
    m_playing_sounds[sound_idx].source->set_pitch(m_pitch);

    m_playing_sounds[sound_idx].name       = std::move(sound_path);
    m_playing_sounds[sound_idx].sound_type = sound_type;

    if (!is_paused) { m_playing_sounds[sound_idx].source->play(); }

    m_playing_sounds[sound_idx].is_free = false;
    m_playing_sounds[sound_idx].source->set_looping(is_looped);

    // ---------- loop through all sounds looking for the one with the same channel --------------
    for (uint16_t j = 0; j < m_playing_sounds.size(); j++) {
        if (j == sound_idx || m_playing_sounds[j].is_free) { continue; }
        if (m_playing_sounds[j].source == m_playing_sounds[sound_idx].source) { free_sound(m_playing_sounds[j]); }
    }

    return id;
}

void SoundService::set_3d_param(TSD_ID id, eSoundMessage msg, void const* data)
{
    if (id.master() || id.index() > m_playing_sounds.size()) { return; }

    auto const& sound = m_playing_sounds[id.index()];

    if (id.stamp() != sound.stamp || !sound.source) { return; }

    switch (msg) {
    case SM_MAX_DISTANCE: {
        float const distance = *static_cast<float const*>(data);
        sound.source->set_max_distance(distance);
    } break;

    case SM_MIN_DISTANCE: {
        float const distance = *static_cast<float const*>(data);
        sound.source->set_min_distance(distance);
    } break;

    case SM_POSITION: {
        auto pos = std::array<float, 3> {};
        std::memcpy(pos.data(), data, sizeof(pos));
        pos[2] = -pos[2];  // Invert Z
        sound.source->set_position_3d(pos);
    } break;
    }
}

void SoundService::sound_restart(TSD_ID /*id*/)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Sound restart !"); }
}

void SoundService::sound_release(TSD_ID /*id*/)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Sound release !"); }
}

void SoundService::set_volume(TSD_ID id, float const volume)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Sound set volume !"); }

    // TODO: separate function
    if (id.master()) {
        for (uint16_t i = 0; i < m_playing_sounds.size(); i++) {
            if (m_playing_sounds[i].is_free) { continue; }

            m_playing_sounds[i].volume = volume;

            float const actual_volume = get_volume_by_type(m_playing_sounds[i]);
            m_playing_sounds[i].source->set_volume_max(actual_volume);
        }
        return;
    }

    if (id.index() >= m_playing_sounds.size()) { return; }

    auto& sound = m_playing_sounds[id.index()];
    if (id.stamp() != sound.stamp || sound.is_free) { return; }

    sound.volume = volume;

    float const actual_volume = get_volume_by_type(sound);
    sound.source->set_volume_max(actual_volume);
}

bool SoundService::is_playing(TSD_ID id)
{
    if (id.master() || id.index() >= m_playing_sounds.size()) { return false; }

    auto& sound = m_playing_sounds[id.index()];

    if (id.stamp() != sound.stamp) { return false; }

    return !sound.is_free;
}

void SoundService::resume(TSD_ID id, int32_t time /* = 0*/)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Resume sound %d", id.index()); }

    if (id.master() || id.index() == 0) {
        for (uint16_t i = 0; i < m_playing_sounds.size(); i++) {
            if (m_playing_sounds[i].is_free) { continue; }
            if (i <= 1) {
                m_playing_sounds[i].source->fade(0.0F, get_volume_by_type(m_playing_sounds[i]), std::chrono::milliseconds(time));
            }

            m_playing_sounds[i].source->play();
        }
        return;
    }

    if (id.index() >= m_playing_sounds.size()) { return; }

    auto& sound = m_playing_sounds[id.index()];
    if (id.stamp() != sound.stamp) { return; }
    if (id.index() <= 1) {
        sound.source->fade(0.0F, get_volume_by_type(sound), std::chrono::milliseconds(time));
        sound.source->play();
    }
}

uint32_t SoundService::get_position(TSD_ID id)
{
    if (id.index() >= m_playing_sounds.size()) { return 0; }

    auto& sound = m_playing_sounds[id.index()];
    if (id.stamp() != sound.stamp) { return 0; }

    auto const pos = sound.source->get_playback_position();
    return static_cast<uint32_t>(pos.count());
}

void SoundService::set_camera_position(const CVECTOR& camera_pos)
{
    m_device->set_listener_position_3d(std::array<float, 3> {camera_pos.x, camera_pos.y, -camera_pos.z});
}

void SoundService::set_camera_orientation(const CVECTOR& nose, const CVECTOR& head)
{
    auto const nose_normalized = !nose;
    auto const head_normalized = !head;

    m_device->set_listener_orientation_3d(
        std::array<float, 3> {nose_normalized.x, nose_normalized.y, -nose_normalized.z},
        std::array<float, 3> {head_normalized.x, head_normalized.y, -head_normalized.z});
}

void SoundService::set_master_volume(float fx_volume, float music_volume, float speech_volume)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Set master volume"); }

    m_fx_volume     = std::clamp(fx_volume, 0.0F, 1.0F);
    m_music_volume  = std::clamp(music_volume, 0.0F, 1.0F);
    m_speech_volume = std::clamp(speech_volume, 0.0F, 1.0F);

    for (uint16_t i = 0; i < m_playing_sounds.size(); ++i) {
        if (m_playing_sounds[i].is_free) { continue; }

        float const actual_volume = get_volume_by_type(m_playing_sounds[i]);
        m_playing_sounds[i].source->set_volume_max(actual_volume);
    }
}

void SoundService::get_master_volume(float& fx_volume, float& music_volume, float& speech_volume)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Get master volume"); }

    fx_volume     = m_fx_volume;
    music_volume  = m_music_volume;
    speech_volume = m_speech_volume;
}

void SoundService::set_pitch(float pitch)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Set pitch"); }

    m_pitch = pitch > std::numeric_limits<float>::epsilon() ? pitch : 0.0F;

    for (uint16_t i = 0; i < m_playing_sounds.size(); i++) {
        if (m_playing_sounds[i].is_free) { continue; }

        m_playing_sounds[i].source->set_pitch(m_pitch);
    }
}

float SoundService::get_pitch()
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Get pitch"); }

    return m_pitch;
}

TSD_ID SoundService::duplicate(TSD_ID /*source_id*/)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Sound duplicate"); }

    return 0;
}

void SoundService::set_enabled(bool is_enabled) {}

void SoundService::set_active_with_fade(bool const is_active)
{
    if (m_fade_time == std::chrono::milliseconds(0)) { return; }

    if (m_device == nullptr) { return; }

    for (auto const& sound: m_playing_sounds) {
        if (sound.is_free) { continue; }

        if (is_active) {
            sound.source->fade(0.0F, get_volume_by_type(sound), m_fade_time);
            sound.source->play();
        } else {
            sound.source->pause();
        }
    }
}

void SoundService::stop(TSD_ID id, int32_t time)
{
    if constexpr (TRACE_INFORMATION) { core.Trace("Stop sound %d", id.index()); }

    // TODO: separate method
    if (id.master()) {
        // --------- remove all sounds -----------------------------------------
        int start = 0;

        // FIXME:
        // for (; time > 0 && start < 2; ++start) {
        //     if (m_playing_sounds[start].is_free) { continue; }

        //     float const vol = m_playing_sounds[start].source->get_volume();
        //     m_playing_sounds[start].source->fade(vol, 0.0F, std::chrono::milliseconds(time));
        // }

        for (uint16_t i = start; i < m_playing_sounds.size(); i++) {
            if (m_playing_sounds[i].is_free) { continue; }

            if (i <= 1) { m_ogg_pos[m_playing_sounds[i].name] = m_playing_sounds[i].source->get_playback_position(); }

            auto const state = m_playing_sounds[i].source->get_state();
            if (state == SourceState::Playing) { m_playing_sounds[i].source->stop(); }

            if constexpr (TRACE_INFORMATION) {
                core.Trace(
                    "m_playing_sounds[%d].channel %p %s state %d",
                    i,
                    m_playing_sounds[i].source.get(),
                    m_playing_sounds[i].name.c_str(),
                    static_cast<int>(state));
            }

            free_sound(m_playing_sounds[i]);
        }

        // --------- remove all sounds -----------------------------------------
        return;
    }

    // Delete the selected
    if (id.index() >= m_playing_sounds.size()) { return; }

    auto& sound = m_playing_sounds[id.index()];

    if (id.stamp() != sound.stamp) { return; }

    if (!sound.source) {
        free_sound(sound);
        return;
    }

    // if (time > 0) {
    //     float const vol = sound.source->get_volume();
    //     sound.source->fade(vol, 0.0F, std::chrono::milliseconds(time));
    // } else {
    if (id.index() <= 1) { m_ogg_pos[sound.name] = sound.source->get_playback_position(); }
    sound.source->stop();

    free_sound(sound);
    // }
}

void SoundService::add_alias(INIFILE& ini_file, std::string_view const& section_name)
{
    if (section_name.empty()) { return; }

    static char temp_string[COMMON_STRING_LENGTH];

    if constexpr (TRACE_INFORMATION) { core.Trace("Add sound alias %s", section_name.data()); }

    m_aliases.emplace(
        section_name,
        Alias {
            .min_distance = ini_file.GetFloat(section_name.data(), "minDistance", -1.0F),
            .max_distance = ini_file.GetFloat(section_name.data(), "maxDistance", -1.0F),
            .volume       = ini_file.GetFloat(section_name.data(), "volume", -1.0F),
        });

    Alias& alias = m_aliases[std::string(section_name)];
    if (ini_file.ReadString(section_name.data(), "name", temp_string, COMMON_STRING_LENGTH, "")) {
        analyse_name_string_and_add_to_alias(alias, temp_string);
        while (ini_file.ReadStringNext(section_name.data(), "name", temp_string, COMMON_STRING_LENGTH)) {
            analyse_name_string_and_add_to_alias(alias, temp_string);
        }
    }
}

void SoundService::load_alias_file(std::string const& filename)
{
    constexpr int const section_name_length = 128;
    static char         section_name[section_name_length];

    std::string ini_name = ALIAS_DIRECTORY;
    ini_name += filename;

    if constexpr (TRACE_INFORMATION) { core.Trace("Find sound alias file %s", ini_name.c_str()); }

    auto alias_ini = fio->OpenIniFile(ini_name.c_str());
    if (!alias_ini) { return; }

    if (alias_ini->GetSectionName(section_name, section_name_length)) {
        add_alias(*alias_ini, section_name);
        while (alias_ini->GetSectionNameNext(section_name, section_name_length)) {
            add_alias(*alias_ini, section_name);
        }
    }
}

void SoundService::init_aliases()
{
    auto const filenames = fio->_GetPathsOrFilenamesByMask(ALIAS_DIRECTORY, "*.ini", false);
    for (auto const& cur_name: filenames) {
        load_alias_file(cur_name);
    }
}

float SoundService::get_volume_by_type(PlayingSound const& sound) const
{
    float volume = sound.volume;
    switch (sound.volume_type) {
    case VOLUME_FX: volume *= m_fx_volume; break;
    case VOLUME_MUSIC: volume *= m_music_volume; break;
    case VOLUME_SPEECH: volume *= m_speech_volume; break;
    default: break;
    }

    return volume;
}

std::shared_ptr<storm::audio::Sound> SoundService::get_from_cache(std::string const& sound_path, eSoundType sound_type)
{
    auto [begin, end] = m_sound_cache.equal_range(sound_path);
    auto entry        = std::find_if(begin, end, [sound_type](auto const& p) { return p.second.sound_type == sound_type; });
    if (entry == end || entry == m_sound_cache.end()) {
        auto sound = m_device->create_sound(sound_path, sound_type == PCM_3D ? Sound::Flags::Spatial3D : Sound::Flags::Stereo2D);
        m_sound_cache.emplace(
            sound_path,
            CacheEntry {
                .sound_type = sound_type,
                .sound      = sound,
            });

        return sound;
    }

    return entry->second.sound;
}

//--------------------------------------------------------------------

bool SoundService::set_scheme(std::string_view const& scheme_name)
{
    reset_scheme();
    return add_scheme(scheme_name);
}

//--------------------------------------------------------------------
void SoundService::reset_scheme()
{
    m_sound_scheme_channels.clear();
}

//--------------------------------------------------------------------
bool SoundService::add_scheme(std::string_view const& scheme_name)
{
    static char temp_string[COMMON_STRING_LENGTH];
    auto        ini = fio->OpenIniFile(SCHEME_INI_NAME);

    if (!ini) { return false; }

    if (ini->ReadString(scheme_name.data(), SCHEME_KEY_NAME, temp_string, COMMON_STRING_LENGTH, "")) {
        add_sound_scheme_channel(temp_string);
        while (ini->ReadStringNext(scheme_name.data(), SCHEME_KEY_NAME, temp_string, COMMON_STRING_LENGTH)) {
            add_sound_scheme_channel(temp_string);
        }
    }

    if (ini->ReadString(scheme_name.data(), SCHEME_KEY_NAME_LOOP, temp_string, COMMON_STRING_LENGTH, "")) {
        add_sound_scheme_channel(temp_string, true);
        while (ini->ReadStringNext(scheme_name.data(), SCHEME_KEY_NAME_LOOP, temp_string, COMMON_STRING_LENGTH)) {
            add_sound_scheme_channel(temp_string, true);
        }
    }

    return true;
}

bool SoundService::add_sound_scheme_channel(std::string_view const& in_string, bool is_looped /*= false*/)
{
    static char temp_string2[COMMON_STRING_LENGTH];
    strncpy_s(temp_string2, in_string.data(), COMMON_STRING_LENGTH);

    char* col = strchr(temp_string2, ',');
    if (col == nullptr) {
        // only name, without delays
        SoundSchemeChannel new_channel;
        new_channel.name           = temp_string2;
        new_channel.min_delay_time = SCHEME_MIN_DELAY;
        new_channel.max_delay_time = SCHEME_MAX_DELAY;
        new_channel.time_to_next_play =
            static_cast<int32_t>(rand(static_cast<float>(new_channel.max_delay_time - new_channel.min_delay_time)));
        new_channel.volume    = 1.F;
        new_channel.is_looped = is_looped;
        if (is_looped) { new_channel.time_to_next_play = 0; }

        m_sound_scheme_channels.push_back(new_channel);
        return true;
    }

    SoundSchemeChannel new_channel;

    // try to convert forthcoming numbers
    int   n1 = 0;
    int   n2 = 0;
    float f1 = 0.0F;

    constexpr int32_t sec_to_ms_mult = 1000;

    int const numbers_converted = sscanf(++col, "%d, %d, %f", &n1, &n2, &f1);
    switch (numbers_converted) {
    case 1:
        new_channel.min_delay_time = SCHEME_MIN_DELAY;
        new_channel.max_delay_time = n1 * sec_to_ms_mult;
        new_channel.volume         = 1.F;
        break;
    case 2:
        new_channel.min_delay_time = n1 * sec_to_ms_mult;
        new_channel.max_delay_time = n2 * sec_to_ms_mult;
        new_channel.volume         = 1.F;
        break;
    case 3:
        new_channel.min_delay_time = n1 * sec_to_ms_mult;
        new_channel.max_delay_time = n2 * sec_to_ms_mult;
        new_channel.volume         = f1;
        break;
    }
    *(--col)              = 0;  // truncate at first ','
    new_channel.name      = temp_string2;
    new_channel.is_looped = is_looped;

    if (is_looped) {
        new_channel.time_to_next_play = 0;
    } else {
        new_channel.time_to_next_play =
            static_cast<int32_t>(rand(static_cast<float>(new_channel.max_delay_time - new_channel.min_delay_time)));
    }

    m_sound_scheme_channels.push_back(new_channel);
    return true;
}

void SoundService::process_sound_schemes()
{
    // handle schemes
    uint32_t const delta_time = core.GetDeltaTime();

    for (size_t i = 0; i < m_sound_scheme_channels.size(); i++) {
        if (m_sound_scheme_channels[i].is_looped) {
            if (m_sound_scheme_channels[i].time_to_next_play != 0) { continue; }
            m_sound_scheme_channels[i].time_to_next_play = -1;
            play(
                m_sound_scheme_channels[i].name,
                PCM_STEREO,
                VOLUME_FX,
                false,
                true,
                0,
                nullptr,
                -1.F,
                -1.F,
                m_sound_scheme_channels[i].volume);
        } else {
            if (m_sound_scheme_channels[i].time_to_next_play > 0) {
                m_sound_scheme_channels[i].time_to_next_play -= delta_time;
            } else {
                m_sound_scheme_channels[i].time_to_next_play = m_sound_scheme_channels[i].min_delay_time
                    + static_cast<int32_t>(rand(
                        static_cast<float>(m_sound_scheme_channels[i].max_delay_time - m_sound_scheme_channels[i].min_delay_time)));
                play(
                    m_sound_scheme_channels[i].name,
                    PCM_STEREO,
                    VOLUME_FX,
                    false,
                    false,
                    0,
                    nullptr,
                    -1.F,
                    -1.F,
                    m_sound_scheme_channels[i].volume);
            }
        }  // looped
    }
}
