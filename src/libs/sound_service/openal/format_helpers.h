#pragma once

#include <AL/al.h>
#include <AL/alext.h>
#include <SDL2/SDL_audio.h>
#include <libs/sound_service/i_audio_backend.h>

namespace storm::audio
{

constexpr SDL_AudioFormat convert_to_sdl_format(IDataStream::Format format)
{
    switch (format) {
    case IDataStream::Format::Unknown: return 0;
    case IDataStream::Format::Int8: return AUDIO_S8;
    case IDataStream::Format::UInt8: return AUDIO_U8;
    case IDataStream::Format::Int16: return AUDIO_S16;
    case IDataStream::Format::UInt16: return AUDIO_U16;
    case IDataStream::Format::Int32: return AUDIO_S32;
    case IDataStream::Format::Float32: return AUDIO_F32;
    }

    return 0;
}

constexpr SDL_AudioFormat convert_to_sdl_format_compatible(IDataStream::Format format)
{
    switch (format) {
    case IDataStream::Format::Unknown: return 0;
    case IDataStream::Format::Int8: [[fallthrough]];
    case IDataStream::Format::UInt8: return AUDIO_U8;  // OpenAL always read 8-bit data as unsigned
    case IDataStream::Format::Int16: [[fallthrough]];
    case IDataStream::Format::UInt16: return AUDIO_S16;  // OpenAL always read 16-bit data as signed
    case IDataStream::Format::Int32: [[fallthrough]];    // OpenAL supports 32-bit data only in float format (and only with extension)
    case IDataStream::Format::Float32:
        if (alIsExtensionPresent("AL_EXT_float32") == AL_TRUE) { return AUDIO_F32; }
        return AUDIO_S16;  // Fallback to 16-bit if 32-bit is not supported
    }

    return 0;
}

constexpr IDataStream::Format convert_format_compatible(IDataStream::Format format)
{
    switch (format) {
    case IDataStream::Format::Unknown: return IDataStream::Format::Unknown;
    case IDataStream::Format::Int8: [[fallthrough]];
    case IDataStream::Format::UInt8: return IDataStream::Format::UInt8;  // OpenAL always read 8-bit data as unsigned
    case IDataStream::Format::Int16: [[fallthrough]];
    case IDataStream::Format::UInt16: return IDataStream::Format::Int16;  // OpenAL always read 16-bit data as signed
    case IDataStream::Format::Int32: [[fallthrough]];  // OpenAL supports 32-bit data only in float format (and only with extension)
    case IDataStream::Format::Float32:
        if (alIsExtensionPresent("AL_EXT_float32") == AL_TRUE) { return IDataStream::Format::Float32; }
        return IDataStream::Format::Int16;  // Fallback to 16-bit if 32-bit is not supported
    }

    return IDataStream::Format::Unknown;
}

constexpr IDataStream::Format convert_from_sdl_format(SDL_AudioFormat format)
{
    switch (format) {
    case AUDIO_S8: return IDataStream::Format::Int8;
    case AUDIO_S16: return IDataStream::Format::Int16;
    case AUDIO_S32: return IDataStream::Format::Int32;
    case AUDIO_U8: return IDataStream::Format::UInt8;
    case AUDIO_U16: return IDataStream::Format::UInt16;
    case AUDIO_F32: return IDataStream::Format::Float32;
    }

    return IDataStream::Format::Unknown;
}

constexpr ALenum convert_to_al_format(IDataStream::Format format, int channels)
{
    switch (format) {
    case IDataStream::Format::Unknown: return 0; break;

    // Base formats
    case IDataStream::Format::Int8: [[fallthrough]];
    case IDataStream::Format::UInt8: return channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8; break;
    case IDataStream::Format::Int16: [[fallthrough]];
    case IDataStream::Format::UInt16: return channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16; break;

    // Extension
    case IDataStream::Format::Int32: [[fallthrough]];
    case IDataStream::Format::Float32:
        if (alIsExtensionPresent("AL_EXT_float32") == AL_FALSE) {
            return 0;
        } else {
            return channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
        }
        break;
    }

    return 0;
}

}  // namespace storm::audio
