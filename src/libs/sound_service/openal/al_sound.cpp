#include "al_sound.h"

#include "al_utils.h"

using namespace storm::audio;

namespace
{

constexpr size_t STREAM_BUFFER_COUNT = 2;

}

struct ALSound::Impl {
    Impl(std::shared_ptr<IDecoder> const& decoder, SoundMode sound_mode) : decoder {decoder}, sound_mode {sound_mode}
    {
        buffers.resize(sound_mode == SoundMode::Stream ? STREAM_BUFFER_COUNT : 1);

        alGenBuffers(static_cast<int>(buffers.size()), buffers.data());
        AL_TRACE_ERRORS();

        SoundFormat sound_format = {};
        decoder->get_sound_format(sound_format);
        switch (sound_format) {
        case SoundFormat::Mono8: format = AL_FORMAT_MONO8; break;
        case SoundFormat::Mono16: format = AL_FORMAT_MONO16; break;
        case SoundFormat::Stereo8: format = AL_FORMAT_STEREO8; break;
        case SoundFormat::Stereo16: format = AL_FORMAT_STEREO16; break;
        }

        decoder->get_sample_rate(sample_rate);

        reset_buffers();
    }

    ~Impl()
    {
        unbind_sources();

        alDeleteBuffers(static_cast<int>(buffers.size()), buffers.data());
        AL_TRACE_ERRORS();
    }

    Result bind_buffers_to_source(unsigned source, bool looping)
    {
        set_looping(source, looping);

        if (sound_mode == SoundMode::WholeFile) {
            alSourcei(source, AL_BUFFER, buffers[0]);
        } else {
            alSourceQueueBuffers(source, static_cast<int>(buffers.size()), buffers.data());
        }

        sources.push_back(source);

        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result unbind_source(unsigned source)
    {
        auto it = std::find_if(sources.begin(), sources.end(), [&source](unsigned const s) { return s == source; });

        if (it == sources.end()) { return Result::ErrInvalidArgument; }

        alSourcei(*it, AL_BUFFER, 0);
        AL_TRACE_ERRORS();

        sources.erase(it);

        return Result::Ok;
    }

    Result set_looping(unsigned source, bool looping) const
    {
        if (sound_mode == SoundMode::WholeFile) {
            alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
        } else {
            alSourcei(source, AL_LOOPING, AL_FALSE);
        }

        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    void bind_buffer_data_whole()
    {
        std::vector<uint8_t> data = {};
        decoder->get_pcm_data(data, true);

        alBufferData(buffers[0], format, data.data(), static_cast<int>(data.size()), sample_rate);
        AL_TRACE_ERRORS();
    }

    void bind_buffer_data_stream()
    {
        for (auto& buffer: buffers) {
            std::vector<uint8_t> data = {};
            decoder->get_pcm_data(data, false);

            alBufferData(buffer, format, data.data(), static_cast<int>(data.size()), sample_rate);
            AL_TRACE_ERRORS();
        }
    }

    bool push_next_data(unsigned buffer, bool is_looping) const
    {
        std::vector<uint8_t> data = {};
        decoder->get_pcm_data(data, false);

        if (data.empty() && is_looping) {
            decoder->seek_start();
            decoder->get_pcm_data(data, false);
        }

        if (data.empty()) { return false; }

        alBufferData(buffer, format, data.data(), static_cast<int>(data.size()), sample_rate);
        AL_TRACE_ERRORS();

        return true;
    }

    void reset_buffers()
    {
        // Unbind buffers from all binded sources
        unbind_sources();

        decoder->seek_start();

        if (sound_mode == SoundMode::WholeFile) {
            bind_buffer_data_whole();
        } else {
            bind_buffer_data_stream();
        }
    }

    void unbind_sources()
    {
        for (auto const source: sources) {
            alSourceStop(source);
            AL_TRACE_ERRORS();

            alSourcei(source, AL_BUFFER, 0);
            AL_TRACE_ERRORS();
        }

        // Clear sources list, as we're not binded to them anymore
        sources.clear();
    }

    std::shared_ptr<IDecoder> decoder;

    SoundMode sound_mode;

    ALenum format;
    int    sample_rate;

    std::vector<unsigned> buffers;
    std::vector<unsigned> sources;
};

ALSound::ALSound(std::shared_ptr<IDecoder> const& decoder, SoundMode sound_mode) : m_impl {std::make_unique<Impl>(decoder, sound_mode)} {}

ALSound::~ALSound() = default;

Result ALSound::get_sound_mode(SoundMode& mode)
{
    mode = m_impl->sound_mode;
    return Result::Ok;
}

Result ALSound::bind_buffers_to_source(unsigned source, bool is_looping)
{
    return m_impl->bind_buffers_to_source(source, is_looping);
}

Result ALSound::unbind_source(unsigned source)
{
    return m_impl->unbind_source(source);
}

Result ALSound::set_looping(unsigned source, bool is_looping)
{
    return m_impl->set_looping(source, is_looping);
}

bool ALSound::push_next_data(unsigned buffer, bool is_looping) const
{
    return m_impl->push_next_data(buffer, is_looping);
}

void ALSound::reset_buffers()
{
    m_impl->reset_buffers();
}
