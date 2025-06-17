#include "sound_service.h"

#include <algorithm>
#include <random>

#include <libs/config/main_config.h>
#include <libs/core/core.h>
#include <libs/filesystem/v_file_service.h>
#include <libs/math/math3d/color.h>
#include <libs/math/math_inlines.h>
#include <libs/math/matrix.h>
#include <libs/util/rands.h>
#include <storm_audio/device.h>
#include <storm_audio/source.h>

using namespace storm::audio;

#define DISTANCEFACTOR 1.0F

namespace
{

constexpr float VOLUME_DEFAULT = 0.5F;
constexpr float PITCH_DEFAULT  = 1.0F;
constexpr float FADE_DEFAULT   = 0.5F;

constexpr size_t STREAM_BUFFER_COUNT = 2;
constexpr size_t BUFFER_SAMPLE_COUNT = 16384;

void trace_message(
    [[maybe_unused]] MessageSeverity severity,
    std::string const&               message,
    std::filesystem::path const&     source_file,
    size_t                           line,
    std::string const&               function_name)
{
    core->Trace("[%s:%zd][%s] %s", source_file.filename().string().c_str(), line, function_name.c_str(), message.c_str());
}

void free_sound(SoundService::PlayingSound& sound)
{
    sound.is_free = true;
    sound.source.reset();
}

}  // namespace

SoundService::SoundService()
    : m_device {nullptr}
    , m_renderer {nullptr}
    , m_is_initialized {false}
    , m_fader_parity {false}
    , m_fx_volume {VOLUME_DEFAULT}
    , m_music_volume {VOLUME_DEFAULT}
    , m_speech_volume {VOLUME_DEFAULT}
    , m_pitch {PITCH_DEFAULT}
    , m_fade_time {std::chrono::milliseconds {0}}
{
    m_thread_should_stop.store(false);
}

SoundService::~SoundService()
{
    m_thread_should_stop.store(true);
    m_update_thread.join();

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

    m_renderer = static_cast<VDX9RENDER*>(core->GetService("RendererService"));
    if (m_renderer == nullptr) { return false; }

    m_device = std::make_unique<Device>(trace_message, Device::DistanceModel::Linear, STREAM_BUFFER_COUNT, BUFFER_SAMPLE_COUNT);
    if (!m_device) { return false; }

    auto const sound_info = storm::main_config::sound_info();
    m_fade_time           = std::chrono::milliseconds(sound_info.fade_time_ms);

    // Reserve first two for music
    m_playing_sounds.resize(2);

    init_aliases();

    m_update_thread = std::thread([&device = *m_device, &should_stop = m_thread_should_stop]() {
        constexpr auto delay = std::chrono::milliseconds(1);
        while (!should_stop.load()) {
            std::this_thread::sleep_for(delay);
            device.update(delay);
        }
    });

    m_is_initialized = true;

    return true;
}

void SoundService::RunEnd() {}

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

    update_playing_list();

    process_sound_schemes();
}

bool SoundService::allocate_sound(SoundID& id)
{
    auto free_sound = std::find_if(
        std::next(m_playing_sounds.begin(), 2), m_playing_sounds.end(), [](PlayingSound const& sound) { return sound.is_free; });
    if (free_sound != m_playing_sounds.end()) {
        id = SoundID::create_id(static_cast<uint16_t>(std::distance(m_playing_sounds.begin(), free_sound)));
        return true;
    }

    if (m_playing_sounds.size() >= std::numeric_limits<uint16_t>::max()) {
        core->Trace("SoundService::allocate_sound(): no empty slots!");
        return false;
    }

    id = SoundID::create_id(static_cast<uint16_t>(m_playing_sounds.size()));
    return true;
}

SoundID SoundService::play(
    std::string const&   name,
    SoundType const      sound_type,
    VolumeType const     volume_type,
    bool const           is_paused /* = false*/,
    bool const           is_looped /* = false*/,
    int32_t const        fade_time /* = 0*/,
    const CVECTOR* const start_position /* = 0*/,
    float const          min_distance /* = -1.0f*/,
    float const          max_distance /* = -1.0f*/,
    float const          volume /* = 1.0f*/)
{
    std::filesystem::path sound_path = fio->base_directory_path(BaseDirectory::Sounds) / name;

    float alias_min_distance = min_distance;
    float alias_max_distance = max_distance;
    float alias_volume       = volume;

    if (m_aliases.contains(name)) {
        auto& alias = m_aliases[name];

        // play sound from the alias ...
        sound_path = fio->base_directory_path(BaseDirectory::Sounds) / alias.files.pickRandom();
        if constexpr (TRACE_INFORMATION) { core->Trace("Play sound from alias %s", sound_path.c_str()); }

        alias_min_distance = alias.min_distance;
        alias_max_distance = alias.max_distance;

        if (alias.volume > std::numeric_limits<float>::epsilon()) { alias_volume = alias.volume; }
    }

    // Normalize and set to lowercase
    sound_path = fio->transform_path(sound_path);

    SoundID const id = sound_type == SoundType::MusicStereo
        ? prepare_music(sound_path.string(), fade_time)
        : prepare_sound(sound_path.string(), sound_type, start_position, alias_min_distance, alias_max_distance);

    if (id == 0) { return 0; }

    uint16_t const sound_idx = id.index();
    auto&          sound     = m_playing_sounds[sound_idx];

    if constexpr (TRACE_INFORMATION) {
        core->Trace(
            "Sound attached, name %s, idx = %d, channel = %p, state = %d",
            sound_path.string().c_str(),
            sound_idx,
            sound.source.get(),
            sound.source->get_state());
    }

    sound.is_free     = false;
    sound.sound_type  = sound_type;
    sound.volume_type = volume_type;
    sound.volume      = alias_volume;
    sound.name        = sound_path.string();

    sound.source->set_volume(get_volume_by_type(sound));

    if (!is_paused) { resume_sound(sound, fade_time); }

    sound.source->set_pitch(m_pitch);
    sound.source->set_looping(is_looped);

    return id;
}

void SoundService::set_3d_param(SoundID id, SoundMessageType msg, void const* data)
{
    if (id.master() || !is_id_valid(id)) { return; }

    auto const& sound = m_playing_sounds[id.index()];
    if (!sound.source || sound.is_free) { return; }

    switch (msg) {
    case SoundMessageType::MaxDistance: {
        float const distance = *static_cast<float const*>(data);
        sound.source->set_max_distance(distance);
    } break;

    case SoundMessageType::MinDistance: {
        float const distance = *static_cast<float const*>(data);
        sound.source->set_min_distance(distance);
    } break;

    case SoundMessageType::Position: {
        auto pos = std::array<float, 3> {};
        std::memcpy(pos.data(), data, sizeof(pos));
        pos[2] = -pos[2];  // Invert Z
        sound.source->set_position_3d(pos);
    } break;
    }
}

void SoundService::sound_restart(SoundID /*id*/)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Sound restart !"); }
}

void SoundService::sound_release(SoundID /*id*/)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Sound release !"); }
}

void SoundService::set_volume(SoundID id, float const volume)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Sound set volume !"); }
    if (!is_id_valid(id)) { return; }

    if (id.master()) {
        set_volume_all(volume);
        return;
    }

    set_volume_for_sound(m_playing_sounds[id.index()], volume);
}

bool SoundService::is_playing(SoundID id)
{
    if (id.master() || !is_id_valid(id)) { return false; }

    return !m_playing_sounds[id.index()].is_free;
}

void SoundService::resume(SoundID id, int32_t time /* = 0*/)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Resume sound %d", id.index()); }
    if (!is_id_valid(id)) { return; }

    if (id.master()) {
        resume_all(time);
        return;
    }

    resume_sound(m_playing_sounds[id.index()], time);
}

uint32_t SoundService::get_position(SoundID id)
{
    if (id.master() || !is_id_valid(id)) { return 0; }

    auto const pos = m_playing_sounds[id.index()].source->get_playback_position();
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
    if constexpr (TRACE_INFORMATION) { core->Trace("Set master volume"); }

    m_fx_volume     = std::clamp(fx_volume, 0.0F, 1.0F);
    m_music_volume  = std::clamp(music_volume, 0.0F, 1.0F);
    m_speech_volume = std::clamp(speech_volume, 0.0F, 1.0F);

    for (auto& sound: m_playing_sounds) {
        if (sound.is_free) { continue; }

        sound.source->set_volume(get_volume_by_type(sound));
    }
}

void SoundService::get_master_volume(float& fx_volume, float& music_volume, float& speech_volume)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Get master volume"); }

    fx_volume     = m_fx_volume;
    music_volume  = m_music_volume;
    speech_volume = m_speech_volume;
}

void SoundService::set_pitch(float pitch)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Set pitch"); }

    m_pitch = pitch > std::numeric_limits<float>::epsilon() ? pitch : 0.0F;

    for (auto& sound: m_playing_sounds) {
        if (sound.is_free) { continue; }

        sound.source->set_pitch(m_pitch);
    }
}

float SoundService::get_pitch()
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Get pitch"); }

    return m_pitch;
}

SoundID SoundService::duplicate(SoundID /*source_id*/)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Sound duplicate"); }

    return 0;
}

void SoundService::set_enabled(bool is_enabled) {}

void SoundService::set_active_with_fade(bool const is_active)
{
    if (m_device == nullptr) { return; }

    for (auto const& sound: m_playing_sounds) {
        if (sound.is_free) { continue; }

        if (is_active) {
            sound.source->play_with_fade(0.0F, get_volume_by_type(sound), m_fade_time);
        } else {
            sound.source->pause();  // No fade on pause
        }
    }
}

void SoundService::stop(SoundID id, int32_t time)
{
    if constexpr (TRACE_INFORMATION) { core->Trace("Stop sound %d", id.index()); }
    if (!is_id_valid(id)) { return; }

    if (id.master()) {
        stop_all(time);
        return;
    }

    stop_sound(m_playing_sounds[id.index()], time);
}

void SoundService::load_alias_file(std::string const& filename)
{
    auto config_file = fio->base_directory_path(BaseDirectory::Aliases) / filename;

    if constexpr (TRACE_INFORMATION) { core->Trace("Find sound alias file %s", config_file.string().c_str()); }

    m_aliases.merge(storm::sound_alias::aliases(config_file));
}

void SoundService::init_aliases()
{
    auto const filenames = fio->string_paths_by_mask(fio->base_directory_path(BaseDirectory::Aliases), "*.ini", false);
    for (auto const& cur_name: filenames) {
        load_alias_file(cur_name);
    }
}

SoundID SoundService::prepare_music(std::string const& name, int32_t fade_time /*= 0*/)
{
    auto const sound = m_device->create_sound_stream(name, Sound::Flags::Stereo2D);
    if (!sound) {
        core->Trace("Error creating sound stream for file %s", name.c_str());
        return 0;
    }

    auto const music_idx     = static_cast<uint16_t>(m_fader_parity);
    m_fader_parity           = !m_fader_parity;
    auto const old_music_idx = static_cast<uint16_t>(m_fader_parity);

    stop(old_music_idx + 1, fade_time);

    uint16_t const sound_idx = music_idx;
    SoundID const  id        = sound_idx + 1;

    auto source = m_device->attach_sound_stream(sound);
    if (!source) { return 0; }

    // Free sounds that binds to same source
    free_playing_on_source(source);
    m_playing_sounds[sound_idx].source = source;

    if (m_ogg_pos.contains(name)) { source->set_playback_position(m_ogg_pos.at(name)); }

    return id;
}

SoundID SoundService::prepare_sound(
    std::string const& name,
    SoundType          sound_type,
    const CVECTOR*     start_position /*= nullptr*/,
    float              min_distance /*= -1.0F*/,
    float              max_distance /*= -1.0F*/)
{
    auto const sound = get_from_cache(name, sound_type);
    if (sound == nullptr) {
        core->Trace("Problem with sound loading !!! '%s'", name.c_str());
        return 0;
    }

    SoundID id = 0;
    if (!allocate_sound(id)) { return 0; }
    uint16_t const sound_idx = id.index();

    if (sound_idx == m_playing_sounds.size()) { m_playing_sounds.push_back({}); }

    auto& playing_sound = m_playing_sounds[sound_idx];
    playing_sound.stamp = id.stamp();

    auto source = m_device->attach_sound(sound);
    if (!source) { return 0; }

    // Free sounds that binds to same source
    free_playing_on_source(source);
    playing_sound.source = source;

    // Adjust parameters for 3D source
    if (sound_type == SoundType::Sound3D) {
        source->set_min_distance(std::max(min_distance, 0.0F) * DISTANCEFACTOR);
        source->set_max_distance(std::max(max_distance, 0.0F) * DISTANCEFACTOR);

        std::array<float, 3> position = {};
        if (start_position != nullptr) {
            position[0] = start_position->x;
            position[1] = start_position->y;
            position[2] = -start_position->z;
        }

        source->set_position_3d(position);
    }

    return id;
}

void SoundService::free_playing_on_source(std::shared_ptr<storm::audio::Source> const& source)
{
    for (auto& sound: m_playing_sounds) {
        if (sound.source == source) { free_sound(sound); }
    }
}

void SoundService::update_playing_list()
{
    for (auto& sound: m_playing_sounds) {
        if (sound.is_free) { continue; }
        if (sound.source->get_state() == SourceState::Free) { free_sound(sound); }
    }
}

void SoundService::set_volume_all(float volume)
{
    for (auto& sound: m_playing_sounds) {
        set_volume_for_sound(sound, volume);
    }
}

void SoundService::set_volume_for_sound(PlayingSound& sound, float volume)
{
    if (sound.is_free) { return; }

    sound.volume = volume;
    sound.source->set_volume(get_volume_by_type(sound));
}

void SoundService::resume_all(int32_t fade_time /*= 0*/)
{
    for (auto& sound: m_playing_sounds) {
        resume_sound(sound, fade_time);
    }
}

void SoundService::resume_sound(PlayingSound& sound, int32_t fade_time /*= 0*/)
{
    if (sound.is_free) { return; }

    if (sound.sound_type == SoundType::MusicStereo) {
        sound.source->play_with_fade(0.0F, get_volume_by_type(sound), std::chrono::milliseconds(fade_time));
    } else {
        sound.source->play();
    }
}

void SoundService::stop_all(int32_t fade_time /*= 0*/)
{
    for (auto& sound: m_playing_sounds) {
        stop_sound(sound, fade_time);
    }
}

void SoundService::stop_sound(PlayingSound& sound, int32_t fade_time /*= 0*/)
{
    if (sound.is_free) { return; }
    if (!sound.source || sound.source->get_state() == SourceState::Free) {
        free_sound(sound);
        return;
    }

    if (sound.sound_type == SoundType::MusicStereo) { m_ogg_pos[sound.name] = sound.source->get_playback_position(); }
    if (fade_time > 0) {
        float const vol = sound.source->get_volume();
        sound.source->stop_with_fade(vol, 0.0F, std::chrono::milliseconds(fade_time));
    } else {
        sound.source->stop();
        free_sound(sound);
    }
}

bool SoundService::is_id_valid(SoundID id)
{
    if (id.master()) { return true; }
    if (id.index() >= m_playing_sounds.size()) { return false; }
    return id.stamp() == m_playing_sounds[id.index()].stamp;
}

float SoundService::get_volume_by_type(PlayingSound const& sound) const
{
    float volume = sound.volume;
    switch (sound.volume_type) {
    case VolumeType::Fx: volume *= m_fx_volume; break;
    case VolumeType::Music: volume *= m_music_volume; break;
    case VolumeType::Speech: volume *= m_speech_volume; break;
    default: break;
    }

    return volume;
}

std::shared_ptr<storm::audio::Sound> SoundService::get_from_cache(std::string const& sound_path, SoundType sound_type)
{
    auto [begin, end] = m_sound_cache.equal_range(sound_path);
    auto entry        = std::find_if(begin, end, [sound_type](auto const& p) { return p.second.sound_type == sound_type; });
    if (entry == end || entry == m_sound_cache.end()) {
        auto sound =
            m_device->create_sound(sound_path, sound_type == SoundType::Sound3D ? Sound::Flags::Spatial3D : Sound::Flags::Stereo2D);
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
    auto        ini = fio->open_ini_file(fio->base_directory_path(BaseDirectory::Ini) / SCHEME_INI_NAME);

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

bool SoundService::add_sound_scheme_channel(std::string const& in_string, bool is_looped /*= false*/)
{
    auto add_scheme_channel = [this, is_looped](std::string const& name, int32_t min_delay, int32_t max_delay, float volume) {
        std::default_random_engine generator;
        // Sometimes min delay is greater than max
        std::uniform_int_distribution<int32_t> distrib(std::min(min_delay, max_delay), std::max(min_delay, max_delay));

        auto const channel = SoundSchemeChannel {
            .name              = name,
            .min_delay_time    = min_delay,
            .max_delay_time    = max_delay,
            .volume            = volume,
            .time_to_next_play = is_looped ? 0 : distrib(generator),
            .is_looped         = is_looped,
        };

        m_sound_scheme_channels.push_back(channel);
    };

    auto const name_end = in_string.find_first_of(',');
    auto const name     = in_string.substr(0, name_end);
    if (name_end == std::string::npos) {
        // If there is only name, use default values
        add_scheme_channel(name, SCHEME_MIN_DELAY, SCHEME_MAX_DELAY, 1.0F);
        return true;
    }

    constexpr int32_t sec_to_ms_mult = 1000;

    // Try to get other parameters from one of possible variations:
    // name, max_delay
    // name, min_delay, max_delay
    // name, min_delay, max_delay, volume
    int   delay1 = 0;
    int   delay2 = 0;
    float volume = 0.0F;

    int const argc = sscanf(in_string.substr(name_end + 1).c_str(), "%d, %d, %f", &delay1, &delay2, &volume);
    switch (argc) {
    case 1: add_scheme_channel(name, SCHEME_MIN_DELAY, delay1 * sec_to_ms_mult, 1.0F); break;
    case 2: add_scheme_channel(name, delay1 * sec_to_ms_mult, delay2 * sec_to_ms_mult, 1.0F); break;
    case 3: add_scheme_channel(name, delay1 * sec_to_ms_mult, delay2 * sec_to_ms_mult, volume); break;
    }

    return true;
}

void SoundService::process_sound_schemes()
{
    // handle schemes
    uint32_t const delta_time = core->GetDeltaTime();

    auto const play_from_channel = [this](auto const& channel, bool const is_looped) {
        play(channel.name, SoundType::SoundStereo, VolumeType::Fx, false, is_looped, 0, nullptr, -1.F, -1.F, channel.volume);
    };

    std::default_random_engine generator;
    for (auto& channel: m_sound_scheme_channels) {
        if (channel.time_to_next_play > 0) {
            channel.time_to_next_play -= delta_time;
            continue;
        }

        // We need to start looping sound only once
        if (channel.is_looped && channel.time_to_next_play == 0) {
            channel.time_to_next_play = -1;
            play_from_channel(channel, true);
            continue;
        }

        // Skip everything that is looping already
        if (channel.is_looped) { continue; }

        // Play sound and choose random delay for next time
        std::uniform_int_distribution<int32_t> distrib(
            std::min(channel.min_delay_time, channel.max_delay_time), std::max(channel.min_delay_time, channel.max_delay_time));
        channel.time_to_next_play = distrib(generator);
        play_from_channel(channel, false);
    }
}
