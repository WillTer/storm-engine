#include "al_sound.h"

#include "al_utils.h"

using namespace storm::audio;

namespace
{

constexpr size_t STREAM_BUFFER_COUNT = 4;

}

struct ALSound::Impl {
    Impl(std::shared_ptr<IDecoder> const& decoder, SoundMode sound_mode) : decoder {decoder}, sound_mode {sound_mode}, is_looping {false}
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
    }

    ~Impl()
    {
        alDeleteBuffers(static_cast<int>(buffers.size()), buffers.data());
        AL_TRACE_ERRORS();
    }

    Result bind_buffers_to_source(unsigned source, bool looping)
    {
        set_looping(source, looping);
        reset_buffers();

        if (sound_mode == SoundMode::WholeFile) {
            alSourcei(source, AL_BUFFER, buffers[0]);
        } else {
            alSourceQueueBuffers(source, static_cast<int>(buffers.size()), buffers.data());
        }

        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    Result set_looping(unsigned source, bool looping)
    {
        is_looping = looping;
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

    bool push_next_data(unsigned buffer) const
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
        decoder->seek_start();

        if (sound_mode == SoundMode::WholeFile) {
            bind_buffer_data_whole();
        } else {
            bind_buffer_data_stream();
        }
    }

    std::shared_ptr<IDecoder> decoder;

    SoundMode sound_mode;

    ALenum format;
    int    sample_rate;

    bool is_looping;

    std::vector<unsigned> buffers;
};

ALSound::ALSound(std::shared_ptr<IDecoder> const& decoder, SoundMode sound_mode) : m_impl {std::make_unique<Impl>(decoder, sound_mode)} {}

ALSound::~ALSound() = default;

Result ALSound::get_sound_mode(SoundMode& mode)
{
    mode = m_impl->sound_mode;
    return Result::Ok;
}

Result ALSound::bind_buffers_to_source(unsigned source, bool looping)
{
    return m_impl->bind_buffers_to_source(source, looping);
}

Result ALSound::set_looping(unsigned source, bool looping)
{
    return m_impl->set_looping(source, looping);
}

bool ALSound::push_next_data(unsigned buffer) const
{
    return m_impl->push_next_data(buffer);
}

void ALSound::reset_buffers()
{
    m_impl->reset_buffers();
}
