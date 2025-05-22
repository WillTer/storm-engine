#include "al_sound.h"

#include "al_utils.h"

using namespace storm::audio;

namespace
{

constexpr size_t STREAM_BUFFER_COUNT      = 2;
constexpr size_t INTERMEDIATE_BUFFER_SIZE = 2048;

}  // namespace

struct ALSound::Impl {
    Impl(std::shared_ptr<IDecoder> const& decoder, ISound::Flags flags) : decoder {decoder}, flags {flags}
    {
        buffers.resize(is_flag_enabled(flags, ISound::Flags::Stream) ? STREAM_BUFFER_COUNT : 1);

        alGenBuffers(static_cast<int>(buffers.size()), buffers.data());
        AL_TRACE_ERRORS();

        decoder->get_sound_format(sound_format);
        switch (sound_format) {
        case SoundFormat::Mono8: al_format = AL_FORMAT_MONO8; break;
        case SoundFormat::Mono16: al_format = AL_FORMAT_MONO16; break;
        case SoundFormat::Stereo8: al_format = AL_FORMAT_STEREO8; break;
        case SoundFormat::Stereo16: al_format = AL_FORMAT_STEREO16; break;
        }

        decoder->get_sample_rate(sample_rate);
        decoder->get_channels(channels);

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

        if (is_flag_enabled(flags, ISound::Flags::Stream)) {
            alSourceQueueBuffers(source, static_cast<int>(buffers.size()), buffers.data());
        } else {
            alSourcei(source, AL_BUFFER, buffers[0]);
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
        if (is_flag_enabled(flags, ISound::Flags::Stream)) {
            alSourcei(source, AL_LOOPING, AL_FALSE);
        } else {
            alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
        }

        AL_TRACE_ERRORS();

        return Result::Ok;
    }

    void bind_buffer_data()
    {
        std::vector<uint8_t> data = {};
        decoder->get_pcm_data(data);

        alBufferData(buffers[0], al_format, data.data(), static_cast<int>(data.size()), sample_rate);
        AL_TRACE_ERRORS();
    }

    void bind_buffer_data_stream()
    {
        for (auto const buffer: buffers) {
            push_next_data(buffer, false);
        }
    }

    bool push_next_data(unsigned buffer, bool is_looping)
    {
        intermediate_buffer.resize(INTERMEDIATE_BUFFER_SIZE);
        auto size = decoder->get_pcm_data(intermediate_buffer);

        if (size == 0 && is_looping) {
            decoder->seek_start();
            size = decoder->get_pcm_data(intermediate_buffer);
        }

        if (size == 0) { return false; }

        alBufferData(buffer, al_format, intermediate_buffer.data(), static_cast<int>(intermediate_buffer.size()), sample_rate);
        AL_TRACE_ERRORS();

        return true;
    }

    void reset_buffers()
    {
        // Unbind buffers from all binded sources
        unbind_sources();

        decoder->seek_start();

        if (is_flag_enabled(flags, ISound::Flags::Stream)) {
            bind_buffer_data_stream();
        } else {
            bind_buffer_data();
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

    ISound::Flags flags;
    SoundFormat   sound_format;

    ALenum al_format;
    int    channels;
    int    sample_rate;

    std::vector<uint8_t> intermediate_buffer;

    std::vector<unsigned> buffers;
    std::vector<unsigned> sources;
};

ALSound::ALSound(std::shared_ptr<IDecoder> const& decoder, ISound::Flags flags) : m_impl {std::make_unique<Impl>(decoder, flags)} {}

ALSound::~ALSound() = default;

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

ISound::Flags ALSound::get_flags() const
{
    return m_impl->flags;
}

int ALSound::get_channels() const
{
    return m_impl->channels;
}

int ALSound::get_sample_rate() const
{
    return m_impl->sample_rate;
}

SoundFormat ALSound::get_sound_format() const
{
    return m_impl->sound_format;
}
