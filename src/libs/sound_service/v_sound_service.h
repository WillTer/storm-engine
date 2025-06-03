#pragma once

#include <limits>
#include <string>
#include <string_view>

#include <libs/core/service.h>
#include <libs/math/c_vector.h>

#include "sound_defines.h"

///////////////////////////////////////////////////////////////////
// DEFINES & TYPES
///////////////////////////////////////////////////////////////////

class SoundID final
{
public:
    SoundID() : m_id(std::numeric_limits<uint32_t>::max()) {}

    SoundID(uint32_t id) : m_id(id) {}

    SoundID(int32_t id) : m_id(id) {}

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

    static SoundID create_id(uint16_t index)
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
    // + in most functions id = 0 equals "apply to all"
    // + play assumes name a plain file name
    ///////////////////////////////////////////////////////////////
    virtual SoundID play(
        std::string const& name,
        SoundType          sound_type,              // sound type
        VolumeType         volume_type,             // volume type
        bool               is_paused      = false,  // cache only, not play
        bool               is_looped      = false,  // looped?
        int32_t            fade_time      = 0,      // fade in, if fade_time > 0
        const CVECTOR*     start_position = nullptr,
        float              min_distance   = -1.0F,
        float              max_distance   = -1.0F,
        float              volume         = 1.0F) = 0;

    virtual SoundID  duplicate(SoundID id)                                            = 0;
    virtual void     set_3d_param(SoundID id, SoundMessageType msg, void const* data) = 0;
    virtual void     stop(SoundID id, int32_t fade_time = 0)                          = 0;
    virtual void     sound_release(SoundID id)                                        = 0;
    virtual void     set_volume(SoundID id, float volume)                             = 0;
    virtual bool     is_playing(SoundID id)                                           = 0;
    virtual uint32_t get_position(SoundID id)                                         = 0;
    virtual void     sound_restart(SoundID id)                                        = 0;
    virtual void     resume(SoundID id, int32_t fade_time = 0)                        = 0;

    // Service functions
    virtual void  set_master_volume(float fx_volume, float music_volume, float speech_volume)    = 0;
    virtual void  get_master_volume(float& fx_volume, float& music_volume, float& speech_volume) = 0;
    virtual void  set_pitch(float pitch)                                                         = 0;
    virtual float get_pitch()                                                                    = 0;
    virtual void  set_camera_position(const CVECTOR& camera_position)                            = 0;
    virtual void  set_camera_orientation(const CVECTOR& nose, const CVECTOR& head)               = 0;

    virtual void reset_scheme()                                  = 0;
    virtual bool set_scheme(std::string_view const& scheme_name) = 0;
    virtual bool add_scheme(std::string_view const& scheme_name) = 0;

    virtual void set_enabled(bool is_enabled)                 = 0;
    virtual void load_alias_file(std::string const& filename) = 0;

    virtual void set_active_with_fade(bool active) = 0;
};

/*
API_SERVICE_START("sound service")
    DECLARE_MAIN_SERVICE(SoundService)
API_SERVICE_END(SoundService)
*/
