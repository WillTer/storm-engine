#include "al_sound.h"

#include <alext.h>

#include "al_utils.h"
#include "format_helpers.h"

using namespace storm::audio;

namespace
{

constexpr size_t STREAM_BUFFER_COUNT      = 2;
constexpr size_t INTERMEDIATE_BUFFER_SIZE = 2048;

}  // namespace

struct ALSound::Impl {
    Impl(std::unique_ptr<IDataStream>&& stream, Flags flags) : m_stream {std::move(stream)}, m_flags {flags}
    {
        m_buffers.resize(is_flag_enabled(flags, Flags::Stream) ? STREAM_BUFFER_COUNT : 1);

        alGenBuffers(static_cast<int>(m_buffers.size()), m_buffers.data());
        AL_TRACE_ERRORS();

        m_sample_rate = m_stream->get_sample_rate();
        m_channels    = m_stream->get_channels();
        m_data_format = m_stream->get_data_format();
        m_al_format   = convert_to_al_format(m_data_format, m_channels);

        reset_buffers();
    }

    ~Impl()
    {
        unbind_sources();

        alDeleteBuffers(static_cast<int>(m_buffers.size()), m_buffers.data());
        AL_TRACE_ERRORS();
    }

    void bind_buffers_to_source(unsigned source, bool looping)
    {
        set_looping(source, looping);

        if (is_flag_enabled(m_flags, ISound::Flags::Stream)) {
            alSourceQueueBuffers(source, static_cast<int>(m_buffers.size()), m_buffers.data());
        } else {
            alSourcei(source, AL_BUFFER, m_buffers[0]);
        }
        AL_TRACE_ERRORS();

        m_binded_sources.push_back(source);
    }

    void unbind_source(unsigned source)
    {
        auto it = std::find_if(m_binded_sources.begin(), m_binded_sources.end(), [&source](unsigned const s) { return s == source; });

        if (it == m_binded_sources.end()) { return; }

        alSourcei(*it, AL_BUFFER, 0);
        AL_TRACE_ERRORS();

        m_binded_sources.erase(it);
    }

    void set_looping(unsigned source, bool looping) const
    {
        if (is_flag_enabled(m_flags, ISound::Flags::Stream)) {
            alSourcei(source, AL_LOOPING, AL_FALSE);
        } else {
            alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
        }
        AL_TRACE_ERRORS();
    }

    void bind_buffer_data()
    {
        std::vector<uint8_t> data = {};
        m_stream->get_pcm_data(data);

        alBufferData(m_buffers[0], m_al_format, data.data(), static_cast<int>(data.size()), m_sample_rate);
        AL_TRACE_ERRORS();
    }

    void bind_buffer_data_stream()
    {
        for (auto const buffer: m_buffers) {
            push_next_data(buffer, false);
        }
    }

    bool push_next_data(unsigned buffer, bool is_looping)
    {
        m_intermediate_buffer.resize(INTERMEDIATE_BUFFER_SIZE);
        auto size = m_stream->get_pcm_data(m_intermediate_buffer);

        if (size == 0 && is_looping) {
            m_stream->seek_start();
            size = m_stream->get_pcm_data(m_intermediate_buffer);
        }

        if (size == 0) { return false; }

        alBufferData(buffer, m_al_format, m_intermediate_buffer.data(), static_cast<int>(m_intermediate_buffer.size()), m_sample_rate);
        AL_TRACE_ERRORS();

        return true;
    }

    void reset_buffers()
    {
        // Unbind buffers from all binded sources
        unbind_sources();

        m_stream->seek_start();

        if (is_flag_enabled(m_flags, ISound::Flags::Stream)) {
            bind_buffer_data_stream();
        } else {
            bind_buffer_data();
        }
    }

    void unbind_sources()
    {
        for (auto const source: m_binded_sources) {
            alSourceStop(source);
            AL_TRACE_ERRORS();

            alSourcei(source, AL_BUFFER, 0);
            AL_TRACE_ERRORS();
        }

        // Clear sources list, as we're not binded to them anymore
        m_binded_sources.clear();
    }

    std::unique_ptr<IDataStream> m_stream;

    Flags               m_flags;
    IDataStream::Format m_data_format;

    ALenum m_al_format;
    int    m_channels;
    int    m_sample_rate;

    std::vector<uint8_t> m_intermediate_buffer;

    std::vector<unsigned> m_buffers;
    std::vector<unsigned> m_binded_sources;
};

ALSound::ALSound(std::unique_ptr<IDataStream>&& stream, ISound::Flags flags) : m_impl {std::make_unique<Impl>(std::move(stream), flags)} {}

ALSound::~ALSound() = default;

void ALSound::bind_buffers_to_source(unsigned source, bool is_looping)
{
    m_impl->bind_buffers_to_source(source, is_looping);
}

void ALSound::unbind_source(unsigned source)
{
    m_impl->unbind_source(source);
}

void ALSound::set_looping(unsigned source, bool is_looping)
{
    m_impl->set_looping(source, is_looping);
}

int ALSound::get_channels() const
{
    return m_impl->m_channels;
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
    return m_impl->m_flags;
}
